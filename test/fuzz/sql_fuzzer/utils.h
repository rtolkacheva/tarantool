#ifndef TARANTOOL_TEST_FUZZ_SQL_FUZZER_UTILS_H
#define TARANTOOL_TEST_FUZZ_SQL_FUZZER_UTILS_H

extern double start_time;

#ifdef __cplusplus
extern "C" 
#endif //#ifdef __cplusplus
void
load_cfg(void);

#ifdef __cplusplus
extern "C" 
#endif //#ifdef __cplusplus
double
tarantool_uptime(void);

void
engine_init(void);

#endif // #ifndef TARANTOOL_TEST_FUZZ_SQL_FUZZER_UTILS_H
