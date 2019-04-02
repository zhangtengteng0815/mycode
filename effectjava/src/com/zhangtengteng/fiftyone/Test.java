package com.zhangtengteng.fiftyone;

import java.util.Calendar;
import java.util.Date;

public class Test {
	public static void main(String[] args) {
		Calendar c=Calendar.getInstance();
		long d1=c.getTimeInMillis();
		String r1="";
		for (int i = 0; i < 100000; i++) {
			r1+=i;
		}
		c=Calendar.getInstance();
		long d2=c.getTimeInMillis();
		System.out.println(d2-d1);
		
		
		StringBuilder b=new StringBuilder();
		for (int i = 0; i < 100000; i++) {
			b.append(i);
		}
		c=Calendar.getInstance();
		long d3=c.getTimeInMillis();
		System.out.println(d3-d2);
	}
}
