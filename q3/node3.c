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
} dt3;

int min_cost3[4];  
int next_hop3[4];
/* students to write the following two routines, and maybe some others */


void rtinit3() {
    // Initialize distance table
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dt3.costs[i][j] = 999;
        }
    }
    
    // Set direct costs
    dt3.costs[0][0] = 7;  // Cost to node 0 via neighbor 0
    dt3.costs[2][2] = 2;  // Cost to node 2 via neighbor 2
    
    // Initialize minimum costs
    min_cost3[0] = 7;
    min_cost3[1] = 999;     // Cost to self is 0
    min_cost3[2] = 2;
    min_cost3[3] = 0;   // Initially unknown
    
    // Send initial distance vector to neighbors (0, 2)
    struct rtpkt pkt0, pkt2;
    
    // Packet to node 0
    pkt0.sourceid = 3;
    pkt0.destid = 0;
    memcpy(pkt0.mincost, min_cost3, sizeof(min_cost3));
    tolayer2(pkt0);
    
    // Packet to node 2
    pkt2.sourceid = 3;
    pkt2.destid = 2;
    memcpy(pkt2.mincost, min_cost3, sizeof(min_cost3));
    tolayer2(pkt2);
    
    printf("rtinit1 called at time %f\n", clocktime);
    printdt3(&dt3);
}



void rtupdate3(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    printf("rtupdate0 called at time %f, received packet from node %d\n", clocktime, source);
    
    // Update distance table with received information
    for (int dest = 0; dest < 4; dest++) {
        dt3.costs[dest][source] = rcvdpkt->mincost[dest] + dt3.costs[source][source];
    }
    
    // Check for new minimum costs
    for (int dest = 0; dest < 4; dest++) {
        if (dest == 0) continue;  // Skip self
        
        int old_min = min_cost3[dest];
        min_cost3[dest] = 999;  // Reset to find new minimum
        
        // Find new minimum cost across all neighbors
        for (int neighbor = 0; neighbor < 4; neighbor++) {
            if (dt3.costs[dest][neighbor] < min_cost3[dest]) {
                min_cost3[dest] = dt3.costs[dest][neighbor];
                next_hop3[dest] = neighbor;
            }
        }
        
        // Check if minimum cost changed
        if (old_min != min_cost3[dest]) {
            updated = 1;
        }
    }
    
    // Print updated distance table
    printdt3(&dt3);
    
    // If any minimum cost changed, send updates to neighbors
    if (updated) {
        struct rtpkt pkt1, pkt2;
        
        // Packet to node 0
        pkt1.sourceid = 3;
        pkt1.destid = 0;
        memcpy(pkt1.mincost, min_cost3, sizeof(min_cost3));
        tolayer2(pkt1);
        printf("Sending update to node 0\n");
        
        // Packet to node 2
        pkt2.sourceid = 3;
        pkt2.destid = 2;
        memcpy(pkt2.mincost, min_cost3, sizeof(min_cost3));
        tolayer2(pkt2);
        printf("Sending update to node 2\n");
        
    }
}


printdt3(dtptr)
  struct distance_table *dtptr;
  
{
  printf("             via     \n");
  printf("   D3 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n",dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 1|  %3d   %3d\n",dtptr->costs[1][0], dtptr->costs[1][2]);
  printf("     2|  %3d   %3d\n",dtptr->costs[2][0], dtptr->costs[2][2]);

}







