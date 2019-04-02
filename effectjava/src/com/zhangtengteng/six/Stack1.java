package com.zhangtengteng.six;

import java.util.Arrays;
import java.util.EmptyStackException;

public class Stack1 {
	private Object[] elements;
	private int size=0;
	private static final int DEFAUL_INITIAL_CAPACITY=16;
	public Stack1(){
		elements=new Object[DEFAUL_INITIAL_CAPACITY];
	}
	public void push(Object e){
		ensureCapacity();
		elements[size++]=e;
	}
	
	public Object pop(){
		if(size==0) throw new EmptyStackException();
		Object result=elements[--size];
		elements[size]=null; //清空过期引用
		return result;
	}
	
	private void ensureCapacity(){
		if(elements.length==size){
			elements=Arrays.copyOf(elements, 2*size+1);
		}
	}
}
