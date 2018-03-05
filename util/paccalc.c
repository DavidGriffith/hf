/*------- ----------------------------------------------------------------
 *          Datendurchsatzberechnung bei PACTOR
 *              ohne/mit Memory-ARQ
 *          Annahme: lineare Superposition
 *                                                        waa 10/90
 *---------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>

double sn,pp,pe,pss,psum,snx,db_min=-20.,db_step,db_max=15.;
double bw=600.,baudr=200.,bwd2br,ln10d10,db,pneu,pa,x;
int n,bits=192;

int main()
{
 register int i,k,maxsum=400;

 bwd2br=bw/baudr/2.;     /*   Bandbreite/(2*Baudrate)         */
 ln10d10=log(10.)/10.;
 printf("\n\nPACCALC berechnet den Datendurchsatz bei Pactor 1.\n");
 printf("PACCALC calculates data throughput for Pactor 1.");
 printf("\nEingabe Anzahl Datenpunkte:\t");
 printf("\nPlease enter number of calculation points:\t");
 scanf ("%d",&n);
 db=db_min;
 db_step=(db_max-db_min)/(n-1);   /* Schrittweite                    */
 printf
     ("\n    Störabstand (dB)     Durchsatz normal (%%)     Durchsatz Memo-ARQ (%%) ");
 printf
     ("\n Signal Noise Ratio(dB)  Throughput normal (%%)    Throughput Memo-ARQ (%%) ");
 printf("\n---------------------------------------------------------\
------------------");
 for (i=0;i<n;i++){
   printf("\n\t % 8.3f",db);
   sn=exp(db*ln10d10);            /* Umrechnung dB -> SN             */
   pe=0.5*exp(-sn*bwd2br);        /* FSK-Bitfehlerwahrscheinlichkeit */
   if (pe<0.1)
     pp=exp(log(1.-pe)*bits);     /* Paket-OK-Wahrscheinlichkeit     */
   else
     pp=0.0;
   pss=pp;
   pp*=100.;
   printf("\t    %8.3f ",pp);
   pa=pss;
   psum=pss;
   for (k=2;k<=maxsum;k++){       /* Iteration fuer M-ARQ            */
     snx=sn*k;                    /* Ann. : lineare Stoerabstands-   */
     if (snx>50.)                 /*   Verbesserung bei Memo-ARQ     */
       pe=0.0;
     else
       pe=0.5*exp(-snx*bwd2br);
     if (pe<0.1)
       pp=exp(log(1.-pe)*bits);
     else
       pp=0.0;
     pneu=(1.-pa)*pss;
     pa+=pneu;
     x=(1.-pa)*pp;
     pa+=x;
     psum+=x/k;
   }
   psum*=100.;
   printf("\t\t%8.3f",psum);
   db+=db_step;
 }
   printf("\n\n\n");
   return (0);
}


