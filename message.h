#ifndef MESSAGE
#define MESSAGE

#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>

#define MSG_JOIN_TO_SERVER 129
#define MSG_NEW_PLAYER 130
#define MSG_SEND_READINESS 131
#define MSG_UPDATE_PLAYER 132
#define MSG_GAME_STARTED 133
#define MSG_SEND_ACTION 134
#define MSG_RESTART_GAME 135
#define MSG_RESPAWN_PLAYER 136
#define MSG_GAME_ENDED 137
#define MSG_PLAYER_LEFT 138
#define MSG_SERVER_DOWN 139
#define MSG_SEND_UDP_PORT 140


struct Header {
    quint16 block_size;
    quint8 msg_type;
};

#define HEADER_SIZE (sizeof(Header))
#define MESSAGE_SIZE (128 - HEADER_SIZE)

struct Msg {
    Header header;
    char data[MESSAGE_SIZE];
};

struct DataToSend {
    quint16 size;
    quint16 ptr;
    char * data;

    DataToSend(char * data, quint16 data_size) {
        ptr = 0;
        size = data_size;
        this->data = new char[data_size];
        memcpy(this->data, data, size);
    }

    DataToSend() {
        ptr = size = 0;
        data = 0;//nullptr;
    }

    void addData(char * data, quint16 data_size) {
        if( data ) {
            if( this->data ) {
                char * new_data = new char[size + data_size];
                memcpy(new_data, this->data, size);
                ptr = size;
                size += data_size;

                for(int i = 0; i < data_size; i++)
                    new_data[ptr++] = data[i];

                delete this->data;
                this->data = new_data;
            } else {
                this->data = new char[data_size];
                size = data_size;
                memcpy(this->data, data, data_size);
            }
        }
    }

    ~DataToSend() {
        if(data)
            delete data;
    }
};

void sendMessage(QTcpSocket * socket, quint8 msg_type, quint16 block_size, const char * data);
void sendDatagram(QUdpSocket * socket, QHostAddress & ip_address, quint16 port, quint8 msg_type, quint16 block_size, const char * data);

#endif // MESSAGE

