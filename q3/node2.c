#include <stdio.h>
#include <string.h>

extern float clocktime;
extern struct rtpkt {
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent 
                         (must be an immediate neighbor) */
  int mincost[4];    /* min cost to node 0 ... 3 */
  };

extern int TRACE;
extern int YES;
extern int NO;

struct distance_table 
{
  int costs[4][4];
} dt2;


int min_cost2[4];  
int next_hop2[4];
/* students to write the following two routines, and maybe some others */


void rtinit2() {
    // Initialize distance table
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dt2.costs[i][j] = 999;
        }
    }
    
    // Set direct costs
    dt2.costs[0][0] = 3;  // Cost to node 0 via neighbor 0
    dt2.costs[1][1] = 1;  // Cost to node 1 via neighbor 1  
    dt2.costs[3][3] = 2;  // Cost to node 3 via neighbor 3  
    
    // Initialize minimum costs
    min_cost2[0] = 3;
    min_cost2[1] = 1;   
    min_cost2[2] = 0;
    min_cost2[3] = 2;   
    
    // Send initial distance vector to neighbors (0, 2)
    struct rtpkt pkt0, pkt1, pkt3;
    
    // Packet to node 0
    pkt0.sourceid = 2;
    pkt0.destid = 0;
    memcpy(pkt0.mincost, min_cost2, sizeof(min_cost2));
    tolayer2(pkt0);
    
    // Packet to node 1
    pkt1.sourceid = 2;
    pkt1.destid = 1;
    memcpy(pkt1.mincost, min_cost2, sizeof(min_cost2));
    tolayer2(pkt1);

    // Packet to node 3
    pkt3.sourceid = 2;
    pkt3.destid = 3;
    memcpy(pkt3.mincost, min_cost2, sizeof(min_cost2));
    tolayer2(pkt3);
    
    printf("rtinit1 called at time %f\n", clocktime);
    printdt1(&dt2);
}


void rtupdate2(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    printf("rtupdate0 called at time %f, received packet from node %d\n", clocktime, source);
    
    // Update distance table with received information
    for (int dest = 0; dest < 4; dest++) {
        dt2.costs[dest][source] = rcvdpkt->mincost[dest] + dt2.costs[source][source];
    }
    
    // Check for new minimum costs
    for (int dest = 0; dest < 4; dest++) {
        if (dest == 0) continue;  // Skip self
        
        int old_min = min_cost2[dest];
        min_cost2[dest] = 999;  // Reset to find new minimum
        
        // Find new minimum cost across all neighbors
        for (int neighbor = 0; neighbor < 4; neighbor++) {
            if (dt2.costs[dest][neighbor] < min_cost2[dest]) {
                min_cost2[dest] = dt2.costs[dest][neighbor];
                next_hop2[dest] = neighbor;
            }
        }
        
        // Check if minimum cost changed
        if (old_min != min_cost2[dest]) {
            updated = 1;
        }
    }
    
    // Print updated distance table
    printdt0(&dt2);
    
    // If any minimum cost changed, send updates to neighbors
    if (updated) {
        struct rtpkt pkt0, pkt1, pkt3;
        
        // Packet to node 1
        pkt1.sourceid = 2;
        pkt1.destid = 1;
        memcpy(pkt1.mincost, min_cost2, sizeof(min_cost2));
        tolayer2(pkt1);
        printf("Sending update to node 1\n");
        
        // Packet to node 0
        pkt0.sourceid = 2;
        pkt0.destid = 0;
        memcpy(pkt0.mincost, min_cost2, sizeof(min_cost2));
        tolayer2(pkt0);
        printf("Sending update to node 0\n");
        
        // Packet to node 3
        pkt3.sourceid = 2;
        pkt3.destid = 3;
        memcpy(pkt3.mincost, min_cost2, sizeof(min_cost2));
        tolayer2(pkt3);
        printf("Sending update to node 3\n");
    }
}



printdt2(dtptr)
  struct distance_table *dtptr;
  
{
  printf("                via     \n");
  printf("   D2 |    0     1    3 \n");
  printf("  ----|-----------------\n");
  printf("     0|  %3d   %3d   %3d\n",dtptr->costs[0][0],
	 dtptr->costs[0][1],dtptr->costs[0][3]);
  printf("dest 1|  %3d   %3d   %3d\n",dtptr->costs[1][0],
	 dtptr->costs[1][1],dtptr->costs[1][3]);
  printf("     3|  %3d   %3d   %3d\n",dtptr->costs[3][0],
	 dtptr->costs[3][1],dtptr->costs[3][3]);
}







