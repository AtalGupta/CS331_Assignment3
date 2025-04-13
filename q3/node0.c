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
} dt0;

int min_cost0[4];  
int next_hop0[4];

/* students to write the following two routines, and maybe some others */

void rtinit0() {
    // Initialize all entries to infinity (999)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dt0.costs[i][j] = 999;
        }
    }
    
    // Set direct costs
    dt0.costs[1][1] = 1;  // Cost to node 1 via neighbor 1
    dt0.costs[2][2] = 3;  // Cost to node 2 via neighbor 2
    dt0.costs[3][3] = 7;  // Cost to node 3 via neighbor 3
    
    // Initialize minimum costs
    min_cost0[0] = 0;     // Cost to self is 0
    min_cost0[1] = 1;
    min_cost0[2] = 3;
    min_cost0[3] = 7;
    
    // Initialize next hops
    next_hop0[1] = 1;
    next_hop0[2] = 2;
    next_hop0[3] = 3;
    
    // Send initial distance vector to neighbors (1, 2, 3)
    struct rtpkt pkt1, pkt2, pkt3;
    
    // Packet to node 1
    pkt1.sourceid = 0;
    pkt1.destid = 1;
    memcpy(pkt1.mincost, min_cost0, sizeof(min_cost0));
    tolayer2(pkt1);
    
    // Packet to node 2
    pkt2.sourceid = 0;
    pkt2.destid = 2;
    memcpy(pkt2.mincost, min_cost0, sizeof(min_cost0));
    tolayer2(pkt2);
    
    // Packet to node 3
    pkt3.sourceid = 0;
    pkt3.destid = 3;
    memcpy(pkt3.mincost, min_cost0, sizeof(min_cost0));
    tolayer2(pkt3);
    
    // Print initialization
    printf("rtinit0 called at time %f\n", clocktime);
    printdt0(&dt0);
}

void rtupdate0(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    printf("rtupdate0 called at time %f, received packet from node %d\n", clocktime, source);
    
    // Update distance table with received information
    for (int dest = 0; dest < 4; dest++) {
        dt0.costs[dest][source] = rcvdpkt->mincost[dest] + dt0.costs[source][source];
    }
    
    // Check for new minimum costs
    for (int dest = 0; dest < 4; dest++) {
        if (dest == 0) continue;  // Skip self
        
        int old_min = min_cost0[dest];
        min_cost0[dest] = 999;  // Reset to find new minimum
        
        // Find new minimum cost across all neighbors
        for (int neighbor = 0; neighbor < 4; neighbor++) {
            if (dt0.costs[dest][neighbor] < min_cost0[dest]) {
                min_cost0[dest] = dt0.costs[dest][neighbor];
                next_hop0[dest] = neighbor;
            }
        }
        
        // Check if minimum cost changed
        if (old_min != min_cost0[dest]) {
            updated = 1;
        }
    }
    
    // Print updated distance table
    printdt0(&dt0);
    
    // If any minimum cost changed, send updates to neighbors
    if (updated) {
        struct rtpkt pkt1, pkt2, pkt3;
        
        // Packet to node 1
        pkt1.sourceid = 0;
        pkt1.destid = 1;
        memcpy(pkt1.mincost, min_cost0, sizeof(min_cost0));
        tolayer2(pkt1);
        printf("Sending update to node 1\n");
        
        // Packet to node 2
        pkt2.sourceid = 0;
        pkt2.destid = 2;
        memcpy(pkt2.mincost, min_cost0, sizeof(min_cost0));
        tolayer2(pkt2);
        printf("Sending update to node 2\n");
        
        // Packet to node 3
        pkt3.sourceid = 0;
        pkt3.destid = 3;
        memcpy(pkt3.mincost, min_cost0, sizeof(min_cost0));
        tolayer2(pkt3);
        printf("Sending update to node 3\n");
    }
}


printdt0(dtptr)
  struct distance_table *dtptr;
  
{
  printf("                via     \n");
  printf("   D0 |    1     2    3 \n");
  printf("  ----|-----------------\n");
  printf("     1|  %3d   %3d   %3d\n",dtptr->costs[1][1],
	 dtptr->costs[1][2],dtptr->costs[1][3]);
  printf("dest 2|  %3d   %3d   %3d\n",dtptr->costs[2][1],
	 dtptr->costs[2][2],dtptr->costs[2][3]);
  printf("     3|  %3d   %3d   %3d\n",dtptr->costs[3][1],
	 dtptr->costs[3][2],dtptr->costs[3][3]);
}

linkhandler0(linkid, newcost)   
  int linkid, newcost;

/* called when cost from 0 to linkid changes from current value to newcost*/
/* You can leave this routine empty if you're an undergrad. If you want */
/* to use this routine, you'll need to change the value of the LINKCHANGE */
/* constant definition in prog3.c from 0 to 1 */
	
{
}

