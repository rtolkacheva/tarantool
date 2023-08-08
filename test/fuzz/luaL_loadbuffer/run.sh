#!/bin/bash

log=log.txt
detailed_log=detailed_log.txt

compile() {
    cmake --build build -t luaL_loadbuffer_fuzzer
}

generate() {
    rm -rf protos
    mkdir protos
    export LPM_DUMP_PROTO_INPUT="1"
    ./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer -runs=10000 -seed=1305068916
    echo files generated: `ls protos | wc -l`
}

check() {
    error_count=0
    snap_restore_count=0
    trace_abort_count=0
    trace_record_count=0

    for file in ./protos/*.test
    do
        echo "Processing $file"
        output=`./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer $file 2>&1 | tail --lines=4`
        line_number=1
        while IFS= read -r line; do
            if [ `echo $line | awk '{print $(NF-1)}'` -gt 0 ]; then
                case "$line_number" in
                    1) ((error_count++));;
                    2) ((snap_restore_count++));;
                    3) ((trace_abort_count++));;
                    4) ((trace_record_count++));;
                esac
            fi
            ((line_number++))
        done <<< "$output"
    done

    echo "Total number of samples: `ls protos/*.test | wc -l`"
    echo "Total number of samples with errors: $error_count"
    echo "Total number of samples with snap restores: $snap_restore_count"
    echo "Total number of samples with abort traces: $trace_abort_count"
    echo "Total number of samples with recorded traces: $trace_record_count"
}

rename() {
    cd ./protos
    for file in [0-9]*.test; do
        # Extract the numeric part of the filename
        num=$(echo "$file" | grep -o '^[0-9]*')
        
        # Pad the numeric part with leading zeros
        padded_num=$(printf "%04d" "$num")
        
        # Extract the extension of the file
        extension="${file##*.}"
        
        # Construct the new filename
        new_filename="${padded_num}.${extension}"
        
        # Rename the file
        mv "$file" "$new_filename"
        
    done
    cd ..
}

check_verbose() {
    export LUA_FUZZER_VERBOSE=1
    echo -n > $log
    echo -n > $detailed_log
    for file in ./protos/*.test
    do
        echo "Processing $file"
        output=`./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer $file 2>&1 | grep -E '^luaL_loadbuffer|^lua_pcall'`
        
        while IFS= read -r line; do
            case "$line" in
                "") ;;
                *) echo $line >> $detailed_log ;;
            esac

            # take only interesting information 
            line=`echo $line | awk -F ': ' '{print $NF}'`

            case "$line" in
                "") ;;
                *\'*\'*) echo $line | awk -F "'" '{print $(NF-2) <name> $NF}' >> $log ;;
                *\'*\'*\'*\'*) echo $line >> $log ;;
                *) echo $line >> $log ;;
            esac
        done <<< "$output"
        # echo $output >> $log
    done
}

process_verbose() {
    sort $log | uniq -c | sort -nr
    echo total error count: `wc $log -l`
}

run_all() {
    compile
    generate
    rename
    check
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        all) SUBCOMMAND=run_all ;;
        compile) SUBCOMMAND=compile ;;
        generate) SUBCOMMAND=generate ;;
        check) SUBCOMMAND=check ;;
        rename) SUBCOMMAND=rename ;;
        check_verbose) SUBCOMMAND=check_verbose ;;
        process_verbose) SUBCOMMAND=process_verbose ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "Running ${SUBCOMMAND}"
$SUBCOMMAND