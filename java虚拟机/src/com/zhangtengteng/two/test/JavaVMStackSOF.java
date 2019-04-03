package com.zhangtengteng.two.test;
/**
 * VM Args:-Xss128k
 * 
 * @author user
 *
 */
public class JavaVMStackSOF {
	private int stackLength=1;
	public void stackLeak(){
		stackLength++;
		stackLeak();
	}
	public static void main(String[] args) throws Throwable {
		JavaVMStackSOF oom=new JavaVMStackSOF();
		try {
			oom.stackLeak();
		} catch (Throwable e) {
			System.out.println("stack length:"+oom.stackLength);
			throw e;
		}
	}
}
