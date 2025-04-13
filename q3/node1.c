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

int connectcosts1[4] = { 1,  0,  1, 999 };

struct distance_table 
{
  int costs[4][4];
} dt1;

int min_cost1[4];  
int next_hop1[4];
/* students to write the following two routines, and maybe some others */


void rtinit1() {
    // Initialize distance table
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dt1.costs[i][j] = 999;
        }
    }
    
    // Set direct costs
    dt1.costs[0][0] = 1;  // Cost to node 0 via neighbor 0
    dt1.costs[2][2] = 1;  // Cost to node 2 via neighbor 2
    
    // Initialize minimum costs
    min_cost1[0] = 1;
    min_cost1[1] = 0;     // Cost to self is 0
    min_cost1[2] = 1;
    min_cost1[3] = 999;   // Initially unknown
    
    // Send initial distance vector to neighbors (0, 2)
    struct rtpkt pkt0, pkt2;
    
    // Packet to node 0
    pkt0.sourceid = 1;
    pkt0.destid = 0;
    memcpy(pkt0.mincost, min_cost1, sizeof(min_cost1));
    tolayer2(pkt0);
    
    // Packet to node 2
    pkt2.sourceid = 1;
    pkt2.destid = 2;
    memcpy(pkt2.mincost, min_cost1, sizeof(min_cost1));
    tolayer2(pkt2);
    
    printf("rtinit1 called at time %f\n", clocktime);
    printdt1(&dt1);
}



void rtupdate1(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    printf("rtupdate0 called at time %f, received packet from node %d\n", clocktime, source);
    
    // Update distance table with received information
    for (int dest = 0; dest < 4; dest++) {
        dt1.costs[dest][source] = rcvdpkt->mincost[dest] + dt1.costs[source][source];
    }
    
    // Check for new minimum costs
    for (int dest = 0; dest < 4; dest++) {
        if (dest == 0) continue;  // Skip self
        
        int old_min = min_cost1[dest];
        min_cost1[dest] = 999;  // Reset to find new minimum
        
        // Find new minimum cost across all neighbors
        for (int neighbor = 0; neighbor < 4; neighbor++) {
            if (dt1.costs[dest][neighbor] < min_cost1[dest]) {
                min_cost1[dest] = dt1.costs[dest][neighbor];
                next_hop1[dest] = neighbor;
            }
        }
        
        // Check if minimum cost changed
        if (old_min != min_cost1[dest]) {
            updated = 1;
        }
    }
    
    // Print updated distance table
    printdt0(&dt1);
    
    // If any minimum cost changed, send updates to neighbors
    if (updated) {
        struct rtpkt pkt1, pkt2;
        
        // Packet to node 0
        pkt1.sourceid = 1;
        pkt1.destid = 0;
        memcpy(pkt1.mincost, min_cost1, sizeof(min_cost1));
        tolayer2(pkt1);
        printf("Sending update to node 0\n");
        
        // Packet to node 2
        pkt2.sourceid = 1;
        pkt2.destid = 2;
        memcpy(pkt2.mincost, min_cost1, sizeof(min_cost1));
        tolayer2(pkt2);
        printf("Sending update to node 2\n");
        
    }
}



printdt1(dtptr)
  struct distance_table *dtptr;
  
{
  printf("             via   \n");
  printf("   D1 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n",dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 2|  %3d   %3d\n",dtptr->costs[2][0], dtptr->costs[2][2]);
  printf("     3|  %3d   %3d\n",dtptr->costs[3][0], dtptr->costs[3][2]);

}



linkhandler1(linkid, newcost)   
int linkid, newcost;   
/* called when cost from 1 to linkid changes from current value to newcost*/
/* You can leave this routine empty if you're an undergrad. If you want */
/* to use this routine, you'll need to change the value of the LINKCHANGE */
/* constant definition in prog3.c from 0 to 1 */
	
{
}


