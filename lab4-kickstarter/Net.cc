#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    int out;
    int hops_horario;
    int hops_antihorario;

    bool BestWay;

    cQueue buffer;

    void sendPacket(Packet *pkt);
    void sendPacketBuff();
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {

    bool BestWay = false;

    int hops_antihorario = 0;
    int hops_horario = 0;

    Packet *pkt_horario = new packet();
    pkt_horario->setSource(this->getParentModule()->getIndex());
    pkt_horario->setDestination(par("destination"));
    pkt_horario->setKind(2);
    pkt_horario->setHopCount(0);
    send(pkt_horario,"toLnk$o",0);

    Packet *pkt_antihorario = new packet();
    pkt_antihorario->setSource(this->getParentModule()->getIndex());
    pkt_antihorario->setDestination(par("destination"));
    pkt_antihorario->setKind(3);
    pkt_antihorario->setHopCount(0);
    send(pkt_antihorario,"toLnk$o",1);

}

void Net::finish() {
}

void Net::sendPacket(Packet *pkt){

    if (hops_antihorario > hops_horario){
        out = 0;
    }
    else if (hops_antihorario < hops_horario){
        out = 1;
    }
    else {
        out = rand() % 2;
    }
    send(pkt, "toLnk$0", out);

}

void Net::sendPacketBuff(){
    while(!(buffer.isEmpty())){
        Packet* pkt = (Packet*) buffer.pop();
        sendPacket(pkt);
    }
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;

    // If this node is the final destination, and packet is type 2 or 3
    if (pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 2){
        hops_horario = pkt->getHopCount();
        delete (pkt);

    } else if (pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 3){
        hops_antihorario = pkt->getHopCount();
        delete (pkt);
    }

    if (hops_antihorario != 0 && hops_horario != 0){
        BestWay = true;
    }
    // If this node is the final destination, send to App
    if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        send(msg, "toApp$o");
    } else if (pkt->arrivedOn("toLnk$i")){
    // If not, forward the packet to some else... to who?
        if (pkt->getKind() == 2 or pkt->getKind() == 3){
            pkt->setHopCount(pkt->getHopCount() + 1);
        }

        if (pkt->arrivedOn("toLnk$i",0)) {
            send(pkt,"toLnk$o",1);
        } else {
            send(pkt,"toLnk$o",0);
        }
    } else {
        assert(pkt->arrivedOn("toApp$i"));

        buffer.insert(pkt);

        if(BestWay){
            sendPacketBuff();
        }
    }
}
