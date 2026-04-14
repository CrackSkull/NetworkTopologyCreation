#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MeshTopology");

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

    // Full mesh connections
    for (uint32_t i = 0; i < 20; i++) {
        for (uint32_t j = i + 1; j < 20; j++) {
            NetDeviceContainer link = p2p.Install(nodes.Get(i), nodes.Get(j));
            interfaces.Add(address.Assign(link));
            address.NewNetwork();
        }
    }

    // Mobility (for animation)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Applications
    UdpEchoServerHelper server(9);
    ApplicationContainer serverApp = server.Install(nodes.Get(19));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    UdpEchoClientHelper client(interfaces.GetAddress(0), 9);
    client.SetAttribute("MaxPackets", UintegerValue(1));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Animation
    AnimationInterface anim("mesh-topology.xml");

    int k = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            anim.SetConstantPosition(nodes.Get(k++), i * 20, j * 20);
        }
    }

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}