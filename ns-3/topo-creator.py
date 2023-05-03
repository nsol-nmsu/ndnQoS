import networkx as nx
#import matplotlib.pyplot as plt
import random
import copy
import sys
import re
from pyvis.network import Network



def main(seed=None):
 ##
 # Parameters for generating network
 ##
 core_nodes = 50   # Total Number of router nodes
 # Num of initial links for nodes (minimum degree of routers)
 initial_links = 2
 max_links = 5
 #num_edge_routers = 30
 c2c_min_hop_distance = 1
 p2c_max_hop_distance = 6
 p2c_min_hop_distance = 1
 p2p_min_hop_distance = 2
 #p2p_max_hop_distance
 current_highest_node = core_nodes
 num_total_clients = 4
 num_routerhelper = 3
 list_edge_routers = []
 list_routerhelper = []
 list_core_routers = []
 list_consumer_routers = []
 list_internal_routers = []
 list_wimax_aps = []
 list_wifi_aps = []
 list_lte_aps = []
 list_of_clients = []
 #variables for creating intial graph
 edges = []
 node_list = []
 node_names = {} #Dictionary for matching node ID in file to generated node number
 router_discription = {}


 # Network
 G = nx.barabasi_albert_graph(core_nodes, initial_links, seed)
 nx.set_node_attributes(G, 'role', 'router')
 nx.set_edge_attributes(G, 'Delay', '2ms')
 nx.set_edge_attributes(G, 'DataRate', '100Mbps')
 nx.set_edge_attributes(G, 'MaxPackets', '1000')
 #sorted_G = sorted(nx.degree(G).items(), key=lambda t: t[1])

 for n in G:
     G.nodes[n]["degree"] = G.degree(n)
     if G.degree(n) == initial_links:
         G.nodes[n]["role"] = "edge-router"
         list_edge_routers.append(n)
     else:
         list_internal_routers.append(n)
     G.nodes[n]["name"] = n 

 current_highest_node = len(G.nodes())
 list_consumer_routers = copy.copy(list_edge_routers)
 list_core_routers = copy.copy(list_internal_routers)
 prefixValue = 1

 #print("Client check:")
 #print(list_consumer_routers)
 #print(list_producers)
 
 # Selecting consumer edge router
 list_consumer_copy = copy.copy(list_consumer_routers)
 for x in list_consumer_copy[:]:
    #print("Size of possible edge routers: " + str(len(list_consumer_routers)))
    for y in list_consumer_routers[:]:
        distance = nx.shortest_path_length(G, source=x, target=y)
        if distance <  c2c_min_hop_distance and x != y:
            list_consumer_routers.remove(y)


 # Selecting proxy
 list_routerhelper = []
 for x in list_core_routers[:]:
    #print("Size of possible proxies: " + str(len(list_core_routers)))
    for z in list_consumer_routers[:]:
        distance = nx.shortest_path_length(G, source=x, target=z)
        if (distance >= p2c_min_hop_distance) and (distance <= p2c_max_hop_distance):
            list_routerhelper.append(x)
            break


 noUnder = False
 while not noUnder:
    noUnder = True
    underCount = {}
    maxUnder = 0
    mostUnder = 0
    list_routerhelper_copy = copy.copy(list_routerhelper)
    for x in list_routerhelper_copy[:]:
        for y in list_routerhelper[:]:
            distance = nx.shortest_path_length(G, source=x, target=y)
            if distance <  p2p_min_hop_distance and x!= y:
                noUnder = False              
                for each in [x, y]:
                    if each not in underCount:
                        underCount[each] = 0
                    underCount[each]+=1
                    if underCount[each] > maxUnder:
                        maxUnder = underCount[each]
                        mostUnder = each
                
    if not noUnder:
        list_routerhelper.remove(mostUnder)
 
 print("After processing core router " + str(x) + 
        " list of possible proxies : " + str(list_routerhelper))
 print("\n\n\n")

 for n in list_routerhelper:
    G.nodes[n]["role"] = "Router Helper"
 
 client_nextHops = {}
 list_client_edge_router= [] 
 for clients in range(num_total_clients):
     # client new node
     G.add_node(current_highest_node,
                role="client",
                name=current_highest_node)
     # select wifi-ap and connect node
    # print("NUmber of clients:")
    # print(list_consumer_routers)
     temp_client_router = random.choice(list_consumer_routers)
     list_client_edge_router.append(temp_client_router)
    # print("Client " + str(current_highest_node) + " connecting to : " +
     #      str(temp_client_router))
     # stats for edge with wifi-ap
     G.add_edge(current_highest_node, temp_client_router,
                Delay="10ms",
                DataRate="5Mbps",
                MaxPackets="1000")
     G.nodes[current_highest_node]["degree"] = G.degree(current_highest_node)
     list_of_clients.append(current_highest_node)
     current_highest_node += 1
 
 #print("Number of nodes in G: " + str(nx.number_of_nodes(G)))

 nt = Network('1080px', '1080px')
 # populates the nodes and edges data structures
 nt.from_nx(G)
 for node in G.nodes():
    if "role" not in nt.nodes[node]:
        nt.nodes[node]["color"] = "grey"
    elif nt.nodes[node]["role"] == "client":
      nt.nodes[node]["color"] = "green"
    elif nt.nodes[node]["role"] == "Router Helper":
        nt.nodes[node]["color"] = "blue"
    else:
        nt.nodes[node]["color"] = "grey"

        
 nt.show('example.html')

 proxies_file = open('BlancTopo.txt', 'w+')

 proxies_file.write("BEG_000\n")
 proxies_file.write(str(current_highest_node)+ "\n")
 proxies_file.write("END_000\n\n\n")

 proxies_file.write("BEG_001\n")
 ip1 = 10
 ip2 = 0
 ip3 = 0
 ip4 = 20
 for x in list(G.edges()):
     ip4+=4
     if ip4 > 255:
        ip4 = 0
        ip3+=1
     fullIP = str(ip1) + "." + str(ip2) + "." + str(ip3) + "." + str(ip4);
     c = G.get_edge_data(x[0], x[1], default=0)
     a = {}
     try:
         c['DataRate']
         a = c
     except:
         for each in c:
             a[c[str(each)]] = each
     proxies_file.write(str(x[0]) + " " +
                      str(x[1]) + " " +
                      str(a['DataRate']) + " " +
                      str(a["Delay"]) + 
                      " " + fullIP + " 255.255.255.252" + "\n")
 proxies_file.write("END_001\n\n\n")

 proxies_file.write("BEG_003\n")
 for each in list_routerhelper:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_003\n\n\n")

 proxies_file.write("BEG_004\n")
 for each in list_of_clients:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_004\n\n\n")

 proxies_file.write("BEG_005\n")
 for x in list(G.edges()):
     c = G.get_edge_data(x[0], x[1], default=0)
     a = {}
     proxies_file.write(str(x[0]) + " " +
                      str(x[1]) + " " +
                      "100\n")
 proxies_file.write("END_005\n\n\n")
 proxies_file.close();

 
if __name__ == "__main__":
	sys.exit(main(*sys.argv[1:]))
