#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RingTopology");

int main() {

    NodeContainer nodes;
    nodes.Create(20);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;

    // Ring connections
    for (uint32_t i = 0; i < 19; i++) {
        NetDeviceContainer link = p2p.Install(nodes.Get(i), nodes.Get(i+1));
        interfaces.Add(address.Assign(link));
        address.NewNetwork();
    }

    // Last node to first node
    NetDeviceContainer lastLink = p2p.Install(nodes.Get(19), nodes.Get(0));
    interfaces.Add(address.Assign(lastLink));

    // Mobility (for NetAnim)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Applications
    UdpEchoServerHelper server(9);
    ApplicationContainer serverApp = server.Install(nodes.Get(19));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    UdpEchoClientHelper client(interfaces.GetAddress(19), 9);
    client.SetAttribute("MaxPackets", UintegerValue(1));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Animation
    AnimationInterface anim("ring-topology.xml");

    for (uint32_t i = 0; i < 20; i++) {
        double angle = i * (2 * M_PI / 20);
        double x = 50 + 30 * cos(angle);
        double y = 50 + 30 * sin(angle);
        anim.SetConstantPosition(nodes.Get(i), x, y);
    }

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}