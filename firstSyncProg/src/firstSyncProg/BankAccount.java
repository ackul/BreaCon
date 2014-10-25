package firstSyncProg;

public class BankAccount {
	int balance = 100;
	
	public synchronized int getBalance()
	{
		return balance;
	}
	
	public synchronized void withdraw(int amount)
	{
		balance = balance -amount;
		if(balance < 0)
		{
			throw new IllegalArgumentException("Amount became Negative Thread problem");
		}
	}
}
	