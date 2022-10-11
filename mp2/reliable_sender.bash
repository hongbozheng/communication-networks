RELIABLE_SENDER=reliable_sender
RECEIVER_HOSTNAME=localhost
PORT=8080
FILENAME=send.txt

if [ "$1" = "" ]; then
    echo "[USAGE]: ./sender.bash <byte_to_xfer>"
    exit
fi

BYTE_TO_XFER=$1

if [ -f "$RELIABLE_SENDER" ]; then
    echo "[INFO]: Executing $RELIABLE_SENDER"
    ./$RELIABLE_SENDER $RECEIVER_HOSTNAME $PORT $FILENAME $BYTE_TO_XFER
else
    echo "[INFO]: Missing executable file $RELIABLE_SENDER"
fi
