#!/bin/bash

# 目标 URL
URL="http://127.0.0.1:1234/"
# 最大并发数
MAX_CONCURRENCY=10000
# 每轮测试时间（秒）
DURATION=10
# 每次增长的并发数
STEP=1000

# 结果输出文件（附带时间戳区分每次测试）
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_FILE="pressure_test_result.txt"
echo "========== Pressure Test Started at $TIMESTAMP ==========" >> $OUTPUT_FILE
echo -e "Concurrency\tRequests/sec\tFailed_requests" >> $OUTPUT_FILE

echo "[+] Starting webbench pressure test for $URL at $(date '+%F %T')"

for ((c=STEP; c<=MAX_CONCURRENCY; c+=STEP))
do
    echo "Testing with concurrency: $c"
    OUTPUT=$(./webbench -c $c -t $DURATION $URL 2>/dev/null)

    SUCCEED=$(echo "$OUTPUT" | grep "Requests:" | awk '{print $2}')
    FAILED=$(echo "$OUTPUT" | grep "Requests:" | awk '{print $4}')

    if [[ -z "$SUCCEED" ]]; then
        echo -e "$c\tERROR\tERROR" >> $OUTPUT_FILE
        echo "  ↳ Error or no response received."
        continue
    fi

    REQ_PER_SEC=$(awk "BEGIN {printf \"%.2f\", $SUCCEED/$DURATION}")


    echo -e "$c\t$REQ_PER_SEC\t$FAILED" >> $OUTPUT_FILE
done

echo "[+] Test finished. Results saved to $OUTPUT_FILE"
