#include "message.h"

void sendMessage(QTcpSocket *socket, quint8 msg_type, quint16 block_size, const char *data) {
    Header header;
    header.msg_type = msg_type;
    header.block_size = block_size;

    DataToSend new_data;

    new_data.addData((char *)&header, HEADER_SIZE);
    new_data.addData((char *)data, block_size);

    socket->write((const char *)new_data.data, new_data.size);
}


void sendDatagram(QUdpSocket *socket, QHostAddress & ip_address, quint16 port, quint8 msg_type, quint16 block_size, const char *data) {
    Header header;
    header.msg_type = msg_type;
    header.block_size = block_size;

    DataToSend new_data;

    new_data.addData((char *)&header, HEADER_SIZE);
    new_data.addData((char *)data, block_size);

    socket->writeDatagram((const char *)new_data.data, new_data.size, ip_address, port);
}
