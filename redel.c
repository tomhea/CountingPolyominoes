#include <time.h>
#include <stdio.h>
#include <strings.h>

#define MAX_SIZE 8
#define SAVE_CALLS 40000000000L

#define X_SIZE    (2 * MAX_SIZE - 2)
#define FULL_SIZE (X_SIZE * MAX_SIZE)

unsigned long long int count [MAX_SIZE+1];
unsigned long long int perstat [MAX_SIZE+1][2*MAX_SIZE+3];
unsigned long long int bbxstat [MAX_SIZE+1][MAX_SIZE+1][MAX_SIZE+1];

#define SET_BOARD(_pos, _value)    board [_pos] = (_value);
#define CHECK_BOARD(_pos, _value)  (board [_pos] == (_value))
#define GET_SUCC(_pos1, _pos2)     _pos2 = untried [_pos1];
#define SET_SUCC(_pos1, _pos2)     untried [_pos1] = _pos2;

#define POS(x,y) (X_SIZE*(y)+(x))

typedef struct {
   int n_neis;
   int neis [4];
} neighbors_t;

typedef enum {
   FREE, OCCUPIED, REACHABLE, DONE
} cell_t;
cell_t      board    [FULL_SIZE];
int         untried  [FULL_SIZE];
neighbors_t all_neis [FULL_SIZE];

int size = 0;
int totper = 1;
int bbx_min_x = MAX_SIZE-2, bbx_max_x = MAX_SIZE-2,
    bbx_min_y = 0, bbx_max_y = 0;
unsigned long long int n_calls = 0;
int n_saves = 0;

void fixed (
   int hdr
);

void print_polyomino (unsigned long long int seq);

#define MAX_STACK 8*(MAX_SIZE-1)
int stack [MAX_STACK];
int p_stack = 0;
#define PUSH(_a, _b, _c, _d) { \
   stack [p_stack] = _a; \
   stack [p_stack + 1] = _b; \
   stack [p_stack + 2] = _c; \
   stack [p_stack + 3] = _d; \
   p_stack += 4; \
   if (p_stack == MAX_STACK+1) {printf ("G@vald!\n"); exit (101);}  \
}
#define POP(_a, _b, _c, _d) { \
   _d = stack [p_stack - 1]; \
   _c = stack [p_stack - 2]; \
   _b = stack [p_stack - 3]; \
   _a = stack [p_stack - 4]; \
   p_stack -= 4; \
}

clock_t clk0;
double ptime;


int main (
   int   argc,
   char* argv []
) {

   int i, j, p, hdr;
   char fn [64];
   FILE* fp;

   /* [Warm restart] */
   if (argc >= 2)
      if (strncmp (argv [1], "-h", 2) == 0) {
         printf ("-h: help\n");
         printf ("-f<file>: continue from data in <file>\n");
         printf ("-p<file>: print data in <file>\n");
         printf ("-n: set MAX_SIZE (nyi)\n");
         return (91);
      } else if (strncmp (argv [1], "-f", 2) == 0 ||
                 strncmp (argv [1], "-p", 2) == 0) {
         FILE* fp = fopen (argv [1] + 2, "rb");
         if (fp == NULL) {
            printf ("fixed: can't open file %s\n", argv [1] + 2);
            return (101);
         }
         fread (& i, sizeof (int), 1, fp);
         if (i != MAX_SIZE) {
            printf ("G@VALD!!!! MAX_SIZE: expected=%d found=%d\n",
                    MAX_SIZE, i);
            return (101);
         }
         fread (& size, sizeof (int), 1, fp);
         fread (& totper, sizeof (int), 1, fp);
         fread (& bbx_min_x, sizeof (int), 1, fp);
         fread (& bbx_max_x, sizeof (int), 1, fp);
         fread (& bbx_min_y, sizeof (int), 1, fp);
         fread (& bbx_max_y, sizeof (int), 1, fp);
         fread (& n_saves, sizeof (int), 1, fp);
         fread (count, sizeof (unsigned long long int), MAX_SIZE + 1, fp);
         fread (perstat, sizeof (unsigned long long int),
                (MAX_SIZE+1)*(2*MAX_SIZE+3), fp);
         fread (bbxstat, sizeof (unsigned long long int),
                   (MAX_SIZE+1)*(MAX_SIZE+1)*(MAX_SIZE+1), fp);
         fread (board, sizeof (cell_t), FULL_SIZE, fp);
         fread (untried, sizeof (int), FULL_SIZE, fp);
         fread (all_neis, sizeof (neighbors_t), FULL_SIZE, fp);
         fread (& hdr, sizeof (int), 1, fp);
         fread (& p_stack, sizeof (int), 1, fp);
         fread (stack, sizeof (int), p_stack, fp);
         fread (& ptime, sizeof (double), 1, fp);
         fclose (fp);
         printf ("fixed: loaded file %s, MAX_SIZE = %d, n_saves = %d\n",
                 argv [1] + 2, MAX_SIZE, n_saves);
         if (strncmp (argv [1], "-p", 2) == 0) {
            int d, h, m, s;
            d = ptime / 86400.0;
            ptime -= d * 86400.0;
            h = ptime / 3600.0;
            ptime -= h * 3600.0;
            m = ptime / 60.0;
            ptime -= m * 60;
            s = ptime;
            printf ("Temporary count (after %dD:%02dH:%02dM:%02dS):\n",
                    d, h, m, s);
            for (p = 1; p <= MAX_SIZE; p ++)
               printf ("Fixed (%d) = %lld\n", p, count [p]);
            return (91);
         }
         clk0 = clock ();
         goto WARM_RESTART;
      } else if (strncmp (argv [1], "-n", 2) == 0) {
      } else {
         printf ("fixed: unknown option %s\n", argv [1]);
         return (101);
      }

   /* [Init] */
   ptime = 0.0;
   clk0 = clock ();
   bzero (count, sizeof (count));
   bzero (perstat, sizeof (perstat));
   bzero (bbxstat, sizeof (bbxstat));
   bzero (untried, sizeof (untried));
   bzero (all_neis, sizeof (all_neis));
   for (j = 0; j <= MAX_SIZE - 2; j ++)
      for (i = - MAX_SIZE + 3; i <= MAX_SIZE - 2; i ++) {
         if (j == 0 && i < 0)
            continue;
         p = j * X_SIZE + i + MAX_SIZE - 2;
            
         all_neis [p].n_neis = 2;
         all_neis [p].neis [0] = p + 1;
         all_neis [p].neis [1] = p + X_SIZE;
         if (j > 0 || (j == 0 && i > 0))
            all_neis [p].neis [all_neis [p].n_neis ++] = p - 1;
         if (j > 1 || (j == 1 && i >= 0))
            all_neis [p].neis [all_neis [p].n_neis ++] = p - X_SIZE;
      }

   /* The algorithm is started with the parent being the empty polyomino, */
   bzero (board, sizeof (board));
   /* and the untried set containing only the origin. */
   hdr = MAX_SIZE - 2;
   WARM_RESTART:
   fixed (hdr);

   /* [Print count] */
   sprintf (fn, "n-fixed-%d-final", MAX_SIZE);
   fp = fopen (fn, "wb");
   if (fp) {
      fprintf (fp, "%d %d\n", 1, MAX_SIZE);
      for (p = 1; p <= MAX_SIZE; p ++) {
         fprintf (fp, "\n%d   %lld\n", p, count [p]);
         fprintf (fp, "   Perimeter\n");
         for (i = 4; i <= 2*MAX_SIZE+2; i++)
            if (perstat[p][i])
               fprintf (fp, "   %d   %lld\n", i, perstat[p][i]);
         fprintf (fp, "   -1\n");
         fprintf (fp, "   Bounding-box\n");
         for (i = 1; i <= MAX_SIZE; i++)
            for (j = 1; j <= MAX_SIZE; j++)
               if (bbxstat[p][i][j])
                  fprintf (fp, "   %d %d   %lld\n", i, j, bbxstat[p][i][j]);
         fprintf (fp, "   -1\n-1\n");
      }
      fprintf (fp, "\n-1\n");
      ptime += (double) (clock () - clk0) / CLOCKS_PER_SEC;
      fprintf (fp,
              "\nTime: %5.2f seconds, %5.2f minutes, %5.2f hours, %5.2f days\n",
               ptime, ptime / 60.0, ptime / 3600.0, ptime / 86400.0);
      fclose (fp);
   }
}


void fixed (
   int hdr
) {

   /* [Local variables] */
   register int cur, save_hdr, n, i, j, addper, savetotper,
            save_bbx_min_x, save_bbx_max_x, save_bbx_min_y, save_bbx_max_y;
   int* p;

   RESTART:

   n_calls ++;
   if (n_calls == SAVE_CALLS) {
      char fn [64];
      FILE* fp;

      n_saves ++;
      sprintf (fn, "n-fixed-%02d-%05d", MAX_SIZE, n_saves);
      fp = fopen (fn, "wb");
      if (fp) {
         int ms;
         ptime += (double) (clock () - clk0) / CLOCKS_PER_SEC;
         ms = MAX_SIZE;
         fwrite (& ms, sizeof (int), 1, fp);
         fwrite (& size, sizeof (int), 1, fp);
         fwrite (& totper, sizeof (int), 1, fp);
         fwrite (& bbx_min_x, sizeof (int), 1, fp);
         fwrite (& bbx_max_x, sizeof (int), 1, fp);
         fwrite (& bbx_min_y, sizeof (int), 1, fp);
         fwrite (& bbx_max_y, sizeof (int), 1, fp);
         fwrite (& n_saves, sizeof (int), 1, fp);
         fwrite (count, sizeof (unsigned long long int), MAX_SIZE + 1, fp);
         fwrite (perstat, sizeof (unsigned long long int),
                 (MAX_SIZE+1)*(2*MAX_SIZE+3), fp);
         fwrite (bbxstat, sizeof (unsigned long long int),
                    (MAX_SIZE+1)*(MAX_SIZE+1)*(MAX_SIZE+1), fp);
         fwrite (board, sizeof (cell_t), FULL_SIZE, fp);
         fwrite (untried, sizeof (int), FULL_SIZE, fp);
         fwrite (all_neis, sizeof (neighbors_t), FULL_SIZE, fp);
         fwrite (& hdr, sizeof (int), 1, fp);
         fwrite (& p_stack, sizeof (int), 1, fp);
         fwrite (stack, sizeof (int), p_stack, fp);
         fwrite (& ptime, sizeof (double), 1, fp);
         fclose (fp);
         printf ("fixed: saved file %s, MAX_SIZE = %d, n_saves = %d\n",
                 fn, MAX_SIZE, n_saves);
         /* exit (1); */
         n_calls = 0;
         clk0 = clock ();
      }
   }

   /* The following steps are repeated until the untried set is exhausted. */
   while (hdr) {

      /* 1. Remove an arbitrary element from the untried set. */
      cur = hdr;
      GET_SUCC (cur, hdr);

       
      /* 1.5 Update perimeter statistics. */
      for (addper = i = 0, p = all_neis [cur].neis;
           i < all_neis [cur].n_neis; i ++) {
         n = * p ++;
         if (CHECK_BOARD (n, FREE))
            addper ++;
      }
       
      if (cur == MAX_SIZE-2)  
         addper += 2;
      else if (cur <= 3*MAX_SIZE-6)  
         addper ++;                  
      if ((cur == X_SIZE-1) || (cur == X_SIZE))  
         addper += 2;                            
      else if (cur == (X_SIZE*(MAX_SIZE-1)+MAX_SIZE-2))  
         addper += 3;
      savetotper = totper;
      totper += addper-1;
      perstat[size+1][totper]++;

       
      /* 1.6 Update bounding-box statistics. */
      i = cur % X_SIZE;
      j = (cur - i) / X_SIZE;
      save_bbx_min_x = bbx_min_x;
      save_bbx_max_x = bbx_max_x;
      save_bbx_min_y = bbx_min_y;
      save_bbx_max_y = bbx_max_y;
      if (bbx_min_x > i)
         bbx_min_x = i;
      else if (bbx_max_x < i)
         bbx_max_x = i;
      if (bbx_min_y > j)
         bbx_min_y = j;
      else if (bbx_max_y < j)
         bbx_max_y = j;
      bbxstat[size+1][bbx_max_x-bbx_min_x+1][bbx_max_y-bbx_min_y+1]++;
 
 

      /* 2. Place a cell at this point. */
      SET_BOARD (cur, OCCUPIED);

      /* 3. Count this new polyomino. */
      count [++ size] ++;

      /* 4. If the size is less than P: */
      if (size < MAX_SIZE) {

         /* (a) Add new neighbors to the untried set. */
         save_hdr = hdr;
         for (i = 0, p = all_neis [cur].neis;
              i < all_neis [cur].n_neis; i ++) {
            n = * p ++;
            if (CHECK_BOARD (n, FREE)) {
               SET_SUCC (n, hdr);
               hdr = n;
               SET_BOARD (n, REACHABLE);
            }
         }

         /* (b) Call this algorithm recursively with the new parent being   */
         /*     the current polyomino, and the new untried set being a copy */
         /*     of the current one.                                         */
          
         PUSH (hdr, save_hdr, cur, savetotper);
         PUSH (save_bbx_min_x, save_bbx_max_x, save_bbx_min_y, save_bbx_max_y);
         goto RESTART;
         RESUME:

         /* (c) Remove the new neighbors from the untried set. */
         while (hdr != save_hdr) {
            SET_BOARD (hdr, FREE);
            GET_SUCC (hdr, hdr);
         }
      }

       
      /* 4.5 Restore perimeter. */
      totper = savetotper;
      /* 4.6 Restore bounding-box. */
      bbx_min_x = save_bbx_min_x;
      bbx_max_x = save_bbx_max_x;
      bbx_min_y = save_bbx_min_y;
      bbx_max_y = save_bbx_max_y;

      /* 5. Remove newest cell. */
      SET_BOARD (cur, DONE);
      size --;
   }

    
   if (p_stack) {
      POP (save_bbx_min_x, save_bbx_max_x, save_bbx_min_y, save_bbx_max_y);
      POP (hdr, save_hdr, cur, savetotper);
      goto RESUME;
   }
}


void print_polyomino (unsigned long long int seq) {

   int x, y, min_x = 1000, max_x = -1000, min_y = 1000, max_y = -1000;

   for (y = 0; y < MAX_SIZE; y++)
      for (x = 0; x < X_SIZE; x++)
         if (CHECK_BOARD (POS(x,y), OCCUPIED)) {
            if (min_x > x) min_x = x;
            if (max_x < x) max_x = x;
            if (min_y > y) min_y = y;
            if (max_y < y) max_y = y;
         }
   printf ("\n(%lld)\n", seq);
   for (y = min_y; y <= max_y; y++) {
      for (x = min_x; x <= max_x; x++)
         printf ("%c", CHECK_BOARD (POS(x,y), OCCUPIED) ? 'X' : ' ');
      printf ("\n");
   }
}
