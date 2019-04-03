package com.zhangtengteng.seven.test;

public class NotInitialization {
	public static void main(String[] args) {
		System.out.println(SubClass.value);
		System.out.println("********************");
		SuperClass[] sca=new SuperClass[10];
		System.out.println("********************");
		//在编译阶段将此常量的值存储到NotInitialization类的常量池中，
		//对ConstClass.HELLOWORLD的引用都转化为NotInitialization类对自身常量池的引用了。
		System.out.println(ConstClass.HELLOWORLD);
		System.out.println("********************");
		System.out.println(SuperClass.value);
		System.out.println(SubClass.value);
	}
}
