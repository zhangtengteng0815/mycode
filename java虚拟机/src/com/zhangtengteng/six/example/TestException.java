package com.zhangtengteng.six.example;

public class TestException {
	public static int inc(){
		int x;
		try{
			x=1;
			return x;
		}catch(Exception e){
			x=2;
			return x;
		}finally{
			x=3;
		}
	}
	public static void main(String[] args) {
		System.out.println(inc());
	}
}
