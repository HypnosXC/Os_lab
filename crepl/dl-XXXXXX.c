int fat(int i) { return i==0 ? 1 :i*fat(i-1);}
int gcd(int a,int b) { return b==0 ? a : gcd(b,a%b);}
int expr_3() { return (fat(2)*gcd(2,4)); }
int expr_4() { return (fat(4)); }
int expr_5() { return (fat(5)*gcd(100,10100)); }
