package firstSyncProg;
import static java.lang.System.out;

public class users extends Thread{
	private BankAccount acc;
		
	 public users(BankAccount acc) {
		super();
		this.acc = acc;
	}
	
	public void run() {
		
		if(acc.getBalance() >= 70)
		{
			acc.withdraw(70);
			out.print("Balance after "+Thread.currentThread().getName()+" withdrawl: "+acc.getBalance()+"\n");
			 
		}
		else
		{
			out.print(Thread.currentThread().getName()+" couldn't execute\n");
		}
		}

}
