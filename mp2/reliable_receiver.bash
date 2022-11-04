FILE=receive.txt
RELIABLE_RECEIVER=reliable_receiver
PORT=8080

if [ -f "$FILE" ]; then
    echo "[INFO]: Destination file $FILE already exists"
    echo "[INFO]: Removing destination file $FILE"
    rm $FILE
    echo "[INFO]: Destination file $FILE removed"
    echo "[INFO]: Creating new destination file $FILE"
    touch $FILE
    echo "[INFO]: Destination file $FILE created"
else
    echo "[INFO]: Missing destination file $FILE"
    echo "[INFO]: Creating destination file $FILE"
    touch $FILE
    echo "[INFO]: Destination file $FILE created"
fi

if [ -f "$RELIABLE_RECEIVER" ]; then
    echo "[INFO]: Executing $RELIABLE_RECEIVER"
    echo "--------------------------------------------------"
    ./$RELIABLE_RECEIVER $PORT $FILE
else
    echo "[INFO]: Missing executable file $RELIABLE_RECEIVER"
fi