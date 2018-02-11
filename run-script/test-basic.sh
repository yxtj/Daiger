


# basic
./go-kernel.sh $ALGORITHM "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"

# delta
../gen/delta-gen.exe $PARTS $GRAPH_FDR

# delta no degree
./go-delta.sh $ALGORITHM $PARTS $GRAPH_FDR $INIT_FDR $DELTA_PREFIX $RESULT_FDR $PORTION $ALPHA $DEGREE $SNAPSHOT $VERB_LVL $HOSTFILE

# delta degree
./go-delta.sh $ALGORITHM $PARTS $GRAPH_FDR $INIT_FDR $DELTA_PREFIX $RESULT_FDR $PORTION $ALPHA $DEGREE $SNAPSHOT $VERB_LVL $HOSTFILE

