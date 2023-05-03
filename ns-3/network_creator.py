import networkx as nx
#import matplotlib.pyplot as plt
import random
import copy
import sys
from pyvis.network import Network



def main(seed=None):
 ##
 # Parameters for generating network
 ##
 core_nodes = 20   # Total Number of router nodes
 # Num of initial links for nodes (minimum degree of routers)
 initial_links = 2
 num_producers = 0
 #num_edge_routers = 30
 b2b_min_hop_distance = 2
 current_highest_node = core_nodes
 num_total_clients = 4
 num_RH = 3
 num_producers_each_client = 1
 list_edge_routers = []
 list_RH = []
 list_servers = []
 list_base = []
 list_core_routers = []
 list_consumer_routers = []
 list_internal_routers = []
 list_producer_routers = []
 list_wimax_aps = []
 list_wifi_aps = []
 list_lte_aps = []
 list_of_clients = []
 list_of_prefixes = []

 seed = int(seed)
 if seed is None:
 	sys.stderr.write("Seed not supplied \n")
 	exit(0)
 
 random.seed(seed)
 
 # Network
 G = nx.barabasi_albert_graph(core_nodes, initial_links, seed)
 nx.set_node_attributes(G, 'role', 'router')
 nx.set_edge_attributes(G, 'Delay', '2ms')
 nx.set_edge_attributes(G, 'DataRate', '100Mbps')
 nx.set_edge_attributes(G, 'MaxPackets', '1000')
 #sorted_G = sorted(nx.degree(G).items(), key=lambda t: t[1])
 
 
 for n in G:
     G.node[n]["degree"] = G.degree(n)
     if G.degree(n) == initial_links:
         G.node[n]["role"] = "edge-router"
         list_edge_routers.append(n)
     else:
         list_internal_routers.append(n)
     G.node[n]["name"] = n
 
 list_consumer_routers = copy.copy(list_edge_routers)
 list_server_routers = copy.copy(list_edge_routers)
 list_base_routers = copy.copy(list_edge_routers)

 list_core_routers = copy.copy(list_internal_routers)
 prefixValue = 1

 # pick producers
 for i in range(len(list_server_routers)):
     x = random.choice(list_server_routers)
     sa = random.randint(1,2)
     for i in range(sa):
         G.add_node(current_highest_node,
                 role="server",
                 prefix=str("/prefix/server"+str(prefixValue)),
                 name=current_highest_node)
         G.add_edge(current_highest_node, x,
                 Delay="2ms",
                 DataRate="500Mbps",
                 MaxPackets="1000")
         G.node[current_highest_node]["degree"] = G.degree(current_highest_node)
         list_servers.append(current_highest_node)
         current_highest_node += 1
         prefixValue += 1

     list_server_routers.remove(x)

 print("Client check:")
 print(list_consumer_routers)
 print(list_servers)

 print("Base", list_base_routers)
 for each in list_core_routers:
     G.node[each]["role"] = "router"
 # pick base stations
 while len(list_base_routers) != 0:
     print(list_base_routers)
     x = random.choice(list_base_routers)
     print(x)
     sa = random.randint(1,2)
     bases = []
     for i in range(sa):
         G.add_node(current_highest_node,
                 role="base",
                 prefix=str("/prefix/base"+str(prefixValue)),
                 name=current_highest_node)
         G.add_edge(current_highest_node, x,
                 Delay="2ms",
                 DataRate="500Mbps",
                 MaxPackets="1000")
         G.node[current_highest_node]["degree"] = G.degree(current_highest_node)
         if i == 1: 
          	G.add_edge(current_highest_node, current_highest_node-1,
                 	Delay="2ms",
               		DataRate="500Mbps",
                 	MaxPackets="1000")

         list_base.append(current_highest_node)
         bases.append(current_highest_node)
         current_highest_node += 1
         prefixValue += 1
     list_base_routers.remove(x)

     for each in list_base_routers:
         print("added",each)
         distance = nx.shortest_path_length(G, source=x, target=each)
         if distance <= 2:
             G.add_node(current_highest_node,
                     role="base",
                     prefix=str("/prefix/base"+str(prefixValue)),
                     name=current_highest_node)
             G.add_edge(current_highest_node, each,
                     Delay="2ms",
                     DataRate="500Mbps",
                     MaxPackets="1000")
             G.node[current_highest_node]["degree"] = G.degree(current_highest_node)
             b = random.choice(bases)
             G.add_edge(b, current_highest_node,
                     Delay="2ms",
                     DataRate="500Mbps",
                     MaxPackets="1000")

             list_base.append(current_highest_node)
             list_base_routers.remove(each)
             current_highest_node += 1
             prefixValue += 1
             break


 print(list_base)
 client_nextHops = {}
 list_client_edge_router= [] 
 list_of_pecs=[]
 for base in list_base:
     clients = []
     pecs = []
     at = random.randint(15,30)
     pecp = float(random.randint(20,30))/100.0
     pcut = float(at) * pecp
     # client new node
     for c in range(at):
         if(c<(at-pcut)):
             r = "client"
             clients.append(current_highest_node)
             list_of_clients.append(current_highest_node)
         else:
             r = "pec"
             pecs.append(current_highest_node)
             list_of_pecs.append(current_highest_node)

         G.add_node(current_highest_node,
                 role=r,
                 name=current_highest_node)
         # select wifi-ap and connect node
         print("Number of clients:")
         print(r+ " " + str(current_highest_node) + " connecting to : " +
                 str(base))
         # stats for edge with wifi-ap
         G.add_edge(current_highest_node, base,
                 Delay="10ms",
                 DataRate="5Mbps",
                 MaxPackets="1000")
         G.node[current_highest_node]["degree"] = G.degree(current_highest_node)
         current_highest_node += 1

     for con in clients:
         for i in range(2):
             x = random.choice(pecs)    
             G.add_edge(con, x,
                     Delay="10ms",
                     DataRate="5Mbps",
                     MaxPackets="1000")

 
 print("Number of nodes in G: " + str(nx.number_of_nodes(G)))
 

 nt = Network('1080px', '1080px')
 # populates the nodes and edges data structures
 nt.from_nx(G)
 for node in G.nodes():
    if nt.nodes[node]["role"] == "server":
      nt.nodes[node]["color"] = "yellow"
    elif nt.nodes[node]["role"] == "base":
        nt.nodes[node]["color"] = "blue"
    elif nt.nodes[node]["role"] == "client":
        nt.nodes[node]["color"] = "green"
    elif nt.nodes[node]["role"] == "pec":
        nt.nodes[node]["color"] = "orange"
    else:
        nt.nodes[node]["color"] = "red"

        
 nt.show('example.html')
 #plt.show()
 proxies_file = open('topo.txt', 'w+')

 proxies_file.write("BEG_000\n")
 proxies_file.write(str(current_highest_node)+ "\n")
 proxies_file.write("END_000\n\n\n")

 proxies_file.write("BEG_001\n")
 for x in list(G.edges()):
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
                      " 10.0.0.24 255.255.255.252" + "\n")
 proxies_file.write("END_001\n\n\n")

 proxies_file.write("BEG_002\n")
 for each in list_servers:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_002\n\n\n")

 proxies_file.write("BEG_003\n")
 for each in list_base:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_003\n\n\n")

 proxies_file.write("BEG_004\n")
 for each in list_of_clients:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_004\n\n\n")

 proxies_file.write("BEG_005\n")
 for each in list_of_pecs:
     proxies_file.write(str(each)+"\n");
 proxies_file.write("END_005\n\n\n")
 proxies_file.close();

 
if __name__ == "__main__":
	sys.exit(main(*sys.argv[1:]))
