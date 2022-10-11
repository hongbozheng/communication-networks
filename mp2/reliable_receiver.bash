FILE=receive.txt
RELIABLE_RECEIVER=reliable_receiver
PORT=8080

if [ -f "$FILE" ]; then
    echo "[INFO]: File $FILE already exists"
    echo "[INFO]: Removing file $FILE"
    rm $FILE
    echo "[INFO]: File $FILE removed"
    echo "[INFO]: Creating new file $FILE"
    touch $FILE
    echo "[INFO]: $FILE created"
else
    echo "[INFO]: Missing file $FILE"
    echo "[INFO]: Creating file $FILE"
    touch $FILE
    echo "[INFO]: $FILE created"
fi

if [ -f "$RELIABLE_RECEIVER" ]; then
    echo "[INFO]: Executing $RELIABLE_RECEIVER"
    ./$RELIABLE_RECEIVER $PORT $FILE
else
    echo "[INFO]: Missing executable file $RELIABLE_RECEIVER"
fi
