package firstSyncProg;

public class threadClass { 
	public static void main(String [] args ) {
		BankAccount b = new BankAccount();
        users t1 = new users(b);
        users t2 = new users(b);
        t1.start();
        t1.setName("T1");
        t2.setName("T2");
        t2.start();
	}
}