int fat(int i) { return i==0 ? 0 :i*fat(i-1);}
int expr_2() { return (fat(2)*fat(3)); }
int gcd(int i,int j) {return j==0 ? i : gcd(j,i%j);}
int expr_4() { return (gcd(12,30)); }
int expr_5() { return (gcd(12,30)*122); }
