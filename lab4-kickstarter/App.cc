#ifndef APP
#define APP

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class App: public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev delayStats;
    cOutVector delayVector;
    cStdDev hops;
    cOutVector PacketGen;
    int packetgenerator;
public:
    App();
    virtual ~App();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(App);

#endif /* APP */

App::App() {
}

App::~App() {
}

void App::initialize() {

    // If interArrivalTime for this node is higher than 0
    // initialize packet generator by scheduling sendMsgEvent
    if (par("interArrivalTime").doubleValue() != 0) {
        sendMsgEvent = new cMessage("sendEvent");
        scheduleAt(par("interArrivalTime"), sendMsgEvent);
    }
    PacketGen.setName("Packets Generated");
    PacketGen.record(0);
    packetgenerator = 0;
    // Initialize statistics
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
    hops.setName("Number of Hops");
}

void App::finish() {
    // Record statistics
    recordScalar("Average delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
    recordScalar("Hops", hops.getMean());
    // Saca un promedio entre los hops de cada paquete. Los pkt provenientes de el nodo 2 hacen 5 saltos, los del nodo 0 hacen 3.
}

void App::handleMessage(cMessage *msg) {

    // if msg is a sendMsgEvent, create and send new packet
    if (msg == sendMsgEvent) {
        // create new packet
        Packet *pkt = new Packet("packet",this->getParentModule()->getIndex());
        pkt->setByteLength(par("packetByteSize"));
        pkt->setSource(this->getParentModule()->getIndex());
        pkt->setDestination(par("destination"));
        pkt->setKind(0);
        // send to net layer
        send(pkt, "toNet$o");

        // compute the new departure time and schedule next sendMsgEvent
        simtime_t departureTime = simTime() + par("interArrivalTime");
        scheduleAt(departureTime, sendMsgEvent);
        packetgenerator++;
        PacketGen.record(packetgenerator);

    }
    // else, msg is a packet from net layer
    else {
        // compute delay and record statistics
        simtime_t delay = simTime() - msg->getCreationTime();
        Packet *pkt = (Packet *) msg;
        delayStats.collect(delay);
        delayVector.record(delay);
        hops.collect(pkt->getHopCount());
        // delete msg
        delete (msg);
    }

}
