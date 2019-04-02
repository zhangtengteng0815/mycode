package com.zhangtengteng.nine;

import java.util.HashMap;
import java.util.Map;

public final class PhoneNumber {
	private final short areaCode;
	private final short prefix;
	private final short lineNumber;
	
	public PhoneNumber(int areaCode,int prefix,int lineNumber) {
		rangeCheck(areaCode,999,"area Code");
		rangeCheck(prefix,999,"prefix");
		rangeCheck(lineNumber,9999,"lineNumber");
		this.areaCode=(short) areaCode;
		this.prefix=(short) prefix;
		this.lineNumber=(short) lineNumber;
	}
	private static void rangeCheck(int arg,int max,String name){
		if (arg<0 || arg>max) {
			throw new IllegalArgumentException(name+":"+arg);
		}
	}
	@Override
	public boolean equals(Object o) {
		if (o==this) {
			return true;
		}
		if (!(o instanceof PhoneNumber)) {
			return false;
		}
		PhoneNumber pn=(PhoneNumber) o;
		return pn.lineNumber==lineNumber
				&&pn.areaCode==areaCode
				&&pn.prefix==prefix;
	}
	
	//the worse possible legal hash function -- never use
//	@Override
//	public int hashCode() {
//		return 42;
//	}
	
	@Override
	public int hashCode() {
		int result=17;
		result=31*result+areaCode;
		result=31*result+prefix;
		result=31*result+lineNumber;
		return result;
	}
	
	@Override
	public String toString() {
		return String.format("(%03d)%03d-%04d", areaCode,prefix,lineNumber);
	}
	
	
	public static void main(String[] args) {
		Map<PhoneNumber, String> m=new HashMap<PhoneNumber, String>();
		m.put(new PhoneNumber(123, 456, 7890), "zhangtengteng");
		System.out.println(m.get(new PhoneNumber(123, 456, 7890)));
		System.out.println(new PhoneNumber(123, 456, 7890));
	}
	
	
}
