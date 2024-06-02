#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include <iostream>

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

    Packet *pkt_horario = new Packet();
    pkt_horario->setSource(this->getParentModule()->getIndex());
    pkt_horario->setDestination(par("destination"));
    pkt_horario->setKind(2);
    pkt_horario->setHopCount(0);
    pkt_horario->setByteLength(par("packetByteSize"));
    send(pkt_horario,"toLnk$o",0);

    Packet *pkt_antihorario = new Packet();
    pkt_antihorario->setSource(this->getParentModule()->getIndex());
    pkt_antihorario->setDestination(par("destination"));
    pkt_antihorario->setKind(3);
    pkt_antihorario->setHopCount(0);
    pkt_antihorario->setByteLength(par("packetByteSize"));
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

    send(pkt, "toLnk$o", out);

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

    // tipo 2 = explorador horario
    // tipo 3 = explorador antihorario
    // tipo 4 = vuelta horario
    // tipo 5 = vuelta antihorario
    // If this node is the final destination, and packet is type 2 or 3
    if (pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 2){
        Packet *back = new Packet();
        back->setSource(this->getParentModule()->getIndex());
        back->setDestination(pkt->getSource());
        back->setKind(4);
        back->setHopCount(0);
        back->setByteLength(par("packetByteSize"));
        delete (pkt);
        pkt = nullptr;
        send(back,"toLnk$o",1);

    } else if (pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 3){
        Packet *back = new Packet();
        back->setSource(this->getParentModule()->getIndex());
        back->setDestination(pkt->getSource());
        back->setKind(5);
        back->setHopCount(0);
        back->setByteLength(par("packetByteSize"));
        delete (pkt);
        pkt = nullptr;
        send(back,"toLnk$o",0);

    } else if(pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 4){
        hops_horario = pkt->getHopCount();
        delete(pkt);
        pkt = nullptr;

    } else if (pkt->getDestination() == this->getParentModule()->getIndex() && pkt->getKind() == 5){
        hops_antihorario = pkt->getHopCount();
        delete(pkt);
        pkt = nullptr;

    }
    if (hops_antihorario != 0 && hops_horario != 0){
        BestWay = true;
    }
    if (pkt != nullptr){
        if (pkt->getDestination() == this->getParentModule()->getIndex()) {
                send(msg, "toApp$o");

        } else if (pkt->arrivedOn("toLnk$i")){

        // If not, forward the packet to some else... to who?
                pkt->setHopCount(pkt->getHopCount() + 1);

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

}
