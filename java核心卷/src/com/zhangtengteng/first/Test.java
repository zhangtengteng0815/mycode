package com.zhangtengteng.first;

import java.nio.charset.Charset;
import java.util.Map;

public class Test {
	public static void main(String[] args) {
		Charset c=Charset.forName("ISO-8859-1");
		//System.out.println(c);
		Map<String, Charset> map=Charset.availableCharsets();
		for (String name:map.keySet()) {
			System.out.println(name);
		}
	}
}
