RELIABLE_SENDER=reliable_sender
RECEIVER_HOSTNAME=localhost
PORT=8080
FILE=send.txt

if [ "$1" = "" ]; then
    echo "[USAGE]: ./sender.bash <byte_to_xfer>"
    exit
fi

if [ ! -f "$FILE" ]; then
    echo "[INFO]: Missing 'send.txt' file to xfer"
    exit
fi

BYTE_TO_XFER=$1

if [ -f "$RELIABLE_SENDER" ]; then
    echo "[INFO]: Executing $RELIABLE_SENDER"
    ./$RELIABLE_SENDER $RECEIVER_HOSTNAME $PORT $FILE $BYTE_TO_XFER
else
    echo "[INFO]: Missing executable file $RELIABLE_SENDER"
fi
