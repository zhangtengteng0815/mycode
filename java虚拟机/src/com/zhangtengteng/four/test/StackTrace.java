package com.zhangtengteng.four.test;

import java.util.Map;

public class StackTrace {
	public static void main(String[] args) {
		for (Map.Entry<Thread, StackTraceElement[]> stackTrace 
				: Thread.getAllStackTraces().entrySet()) {
			Thread thread=stackTrace.getKey();
			StackTraceElement[] stack=stackTrace.getValue();
			if (thread.equals(thread.currentThread())) {
				continue;
			}
			System.out.println("线程："+thread.getName());
			for (StackTraceElement element : stack) {
				System.out.println(element);
			}
		}
	}
}
