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
 RH = 4 # Total Number of router helper nodes
 neigborhood_size  = 4
 neigborhood_hops = 4
 min_c2r_hops = 4
 # Num of initial links for nodes (minimum degree of routers)
 initial_links = 2
 max_links = 3
 num_total_clients = 4
 list_routerhelper = []
 list_of_clients = []
 #variables for creating intial graph
 node_names = {} #Dictionary for matching node ID in file to generated node number


 # Network
 G = nx.Graph()
 count = 0
 list_neigborhood_edge = {}
 list_routerhelper = []
 current_highest_node = 0
 random.seed(seed)

 while count < RH:
    G.add_node(current_highest_node,
                 role="Router Helper",
                 name=current_highest_node)
    list_routerhelper.append(current_highest_node)
    current_highest_node+=1
    count+=1
 
 for router_helper in list_routerhelper:
    level = 0
    node_level_list = []
    while level < neigborhood_hops:
        nodes = 0
        node_level_list.append([])
        while nodes < neigborhood_size:
            G.add_node(current_highest_node,
                 role="core",
                 name=current_highest_node)
            node_level_list[level].append(current_highest_node)
            current_highest_node+=1
            nodes+=1
        if level == 0:   
            for n in node_level_list[level]:
                G.add_edge(n, router_helper,
                    Delay="10ms",
                    DataRate="5Mbps",
                    MaxPackets="1000")
                node_list = []
                for each in node_level_list[level]:
                    if each != n:
                        node_list.append(each)   
                x = random.choice(node_list)
                #G.add_edge(n, x,
                #    Delay="10ms",
                #    DataRate="5Mbps",
                #    MaxPackets="1000")  
        else:             
            for n in node_level_list[level]:
                degree = 100
                x = 0
                node_list_copy = copy.copy(node_level_list[level-1])
                links = random.choice([1,2,2,2])
                while links > 0:
                    links-=1
                    while(degree >= max_links):
                        node_list = []
                        x = random.choice(node_level_list[level-1])
                        degree = G.degree(x)
                        for each in node_list_copy:
                            if each != x:
                                node_list.append(each)   
                        node_list_copy = node_list
                        if len(node_list) == 0:
                            x = router_helper              
                            degree = 0
                    G.add_edge(n, x,
                        Delay="10ms",   
                        DataRate="5Mbps",
                        MaxPackets="1000")                    
                    
                #node_list_copy = copy.copy(node_level_list[level])
                for each in node_level_list[level]:
                    if each != n:
                        node_list_copy.append(each)
                degree = max_links
                links = random.choice([0,1,1])
                while links > 0:
                    links-=1
                    while (degree >= max_links): 
                        node_list = []                                   
                        x = random.choice(node_list_copy)
                        for each in node_list_copy:
                            if each != x:
                                node_list.append(each)   
                        node_list_copy = node_list
                        degree = G.degree(x)                                
                        if len(node_list) == 0:
                            x = router_helper              
                            degree = 0                        
              
                    G.add_edge(n, x,
                        Delay="10ms",   
                        DataRate="5Mbps",
                        MaxPackets="1000")                                   
        level+=1          
        if level == neigborhood_hops:
            print(node_level_list[level-1], router_helper)
            list_neigborhood_edge[router_helper] = copy.copy(node_level_list[level-1])

 list_client_edge_router = []
 for n in range(RH, current_highest_node):
    list_client_edge_router.append(n)

 list_client_edge_router_copy = copy.copy(list_client_edge_router)
 for n in list_client_edge_router_copy:
    for rh in list_routerhelper:
        if not nx.has_path(G, source=n, target=rh):
            continue
        distance = nx.shortest_path_length(G, source=n, target=rh)
        if distance < min_c2r_hops:
            list_client_edge_router.remove(n)

 for n in list_client_edge_router:
    G.nodes[n]["role"] = "client"

 connect_list = []
 for edge in list_neigborhood_edge:
    print(list_neigborhood_edge[edge])
    connect_list.append(random.choice(list_neigborhood_edge[edge]))
 connect_list = []    

 connect_list_copy = copy.copy(connect_list)
 for n in connect_list:
        node_list = []
        for each in connect_list_copy:
            if each != n:
                node_list.append(each)   
        if len(node_list) == 0:
            continue
        connect_list_copy = copy.copy(node_list)  
        x = random.choice(node_list)
        print(n,x)
        G.add_edge(n, x,
            Delay="10ms",   
            DataRate="5Mbps",
            MaxPackets="1000")  

 current_highest_node = len(G.nodes())
 
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
