#include "utils.h"

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sysexits.h>

#include "box/box.h"
#include "box/flightrec.h"
#include "box/memtx_tx.h"
#include "cfg.h"
#include <libutil.h>
#include <say.h>
#include "small/small_features.h"
#include "trivia/util.h"
#include "version.h"

double start_time;

void
load_cfg(void)
{
	const char *work_dir = cfg_gets("work_dir");
	if (work_dir != NULL && chdir(work_dir) == -1)
		panic_syserror("can't chdir to `%s'", work_dir);

	const char *username = cfg_gets("username");
	if (username != NULL) {
		if (getuid() == 0 || geteuid() == 0) {
			struct passwd *pw;
			errno = 0;
			if ((pw = getpwnam(username)) == 0) {
				if (errno) {
					say_syserror("getpwnam: %s",
						     username);
				} else {
					say_error("User not found: %s",
						  username);
				}
				exit(EX_NOUSER);
			}
			if (setgid(pw->pw_gid) < 0 || setgroups(0, NULL) < 0 ||
			    setuid(pw->pw_uid) < 0 || seteuid(pw->pw_uid)) {
				say_syserror("setgid/setuid");
				exit(EX_OSERR);
			}
		} else {
			say_error("can't switch to %s: i'm not root",
				  username);
		}
	}

	if (cfg_geti("coredump")) {
		struct rlimit c = { 0, 0 };
		if (getrlimit(RLIMIT_CORE, &c) < 0) {
			say_syserror("getrlimit");
			exit(EX_OSERR);
		}
		c.rlim_cur = c.rlim_max;
		if (setrlimit(RLIMIT_CORE, &c) < 0) {
			say_syserror("setrlimit");
			exit(EX_OSERR);
		}
#if defined(TARGET_OS_LINUX) && defined(HAVE_PRCTL_H)
		if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) < 0) {
			say_syserror("prctl");
			exit(EX_OSERR);
		}
#endif
	}

	/*
	 * If we're requested to strip coredump
	 * make sure we can do that, otherwise
	 * require user to not turn it on.
	 */
	if (cfg_geti("strip_core")) {
		if (!small_test_feature(SMALL_FEATURE_DONTDUMP)) {
			static const char strip_msg[] =
				"'strip_core' is set but unsupported";
#ifdef TARGET_OS_LINUX
			/*
			 * Linux is known to support madvise(DONT_DUMP)
			 * feature, thus warn on this platform only. The
			 * rest should be notified on verbose level only
			 * to not spam a user.
			 */
			say_warn(strip_msg);
#else
			say_verbose(strip_msg);
#endif
		}
	}

	int background = cfg_geti("background");
	const char *log = cfg_gets("log");
	const char *log_format = cfg_gets("log_format");
	char *pid_file = (char *)cfg_gets("pid_file");
	if (pid_file != NULL) {
		pid_file = abspath(pid_file);
		if (pid_file == NULL)
			panic("out of memory");
	}

	if (background) {
		if (log == NULL) {
			say_crit(
				"'background' requires "
				"'log' configuration option to be set");
			exit(EXIT_FAILURE);
		}
		if (pid_file == NULL) {
			say_crit(
				"'background' requires "
				"'pid_file' configuration option to be set");
			exit(EXIT_FAILURE);
		}
	}

	/*
	 * pid file check must happen before logger init in order for the
	 * error message to show in stderr
	 */
    static struct pidfh *pid_file_handle = NULL;
	if (pid_file != NULL) {
		pid_t other_pid = -1;
		pid_file_handle = pidfile_open(pid_file, 0644, &other_pid);
		if (pid_file_handle == NULL) {
			if (errno == EEXIST) {
				say_crit(
					"the daemon is already running: PID %d",
					(int)other_pid);
			} else {
				say_syserror(
					"failed to create pid file '%s'",
					pid_file);
			}
			exit(EXIT_FAILURE);
		}
	}

	/*
	 * logger init must happen before daemonising in order for the error
	 * to show and for the process to exit with a failure status
	 */
	say_logger_init(log,
			cfg_geti("log_level"),
			cfg_getb("log_nonblock"),
			log_format,
			background);

	/*
	 * Initialize flight recorder after say logger: we might set on_log
	 * callback to duplicate logs to flight recorder.
	 */
	bool flightrec_is_enabled = cfg_getb("flightrec_enabled") == 1;
	if (flightrec_is_enabled) {
		flightrec_init(cfg_gets("memtx_dir"),
			       cfg_geti64("flightrec_logs_size"),
			       cfg_geti64("flightrec_logs_max_msg_size"),
			       cfg_geti("flightrec_logs_log_level"),
			       cfg_getd("flightrec_metrics_interval"),
			       cfg_geti("flightrec_metrics_period"),
			       cfg_geti("flightrec_requests_size"),
			       cfg_geti("flightrec_requests_max_req_size"),
			       cfg_geti("flightrec_requests_max_res_size"));
	}

	memtx_tx_manager_use_mvcc_engine = cfg_getb("memtx_use_mvcc_engine");

	/*
	 * after (optional) daemonising to avoid confusing messages with
	 * different pids
	 */
	say_info("%s %s", tarantool_package(), tarantool_version());
	say_info("log level %i", cfg_geti("log_level"));

	if (pid_file_handle != NULL) {
		if (pidfile_write(pid_file_handle) == -1)
			say_syserror("failed to update pid file '%s'", pid_file);
	}

	box_cfg();
}

double
tarantool_uptime(void)
{
	return ev_monotonic_now(loop()) - start_time;
}
