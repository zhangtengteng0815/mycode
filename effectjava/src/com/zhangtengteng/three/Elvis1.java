package com.zhangtengteng.three;

public class Elvis1 {
	private static final Elvis1 INSTANCE=new Elvis1();
	private Elvis1(){}
	public static Elvis1 getInstance(){
		return INSTANCE;
	}
	
}
