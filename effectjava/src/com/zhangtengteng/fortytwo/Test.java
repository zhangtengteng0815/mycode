package com.zhangtengteng.fortytwo;

import java.util.Arrays;

public class Test {
	static int sum(int...args){
		int sum=0;
		for (int arg : args)
			sum+=arg;
		return sum;
	}
	
	public void foo(){}
	public void foo(int a1){}
	public void foo(int a1, int a2){}
	public void foo(int a1, int a2 ,int a3){}
	public void foo(int a1, int a2 ,int a3, int...rest){}
	
	public static void main(String[] args) {
		System.out.println(sum(1,2,3,4,5));
		Arrays.asList();
	}
}
