package com.zhangtengteng.thirtyeight;

import java.math.BigInteger;

public class Test {
	/**
	 * @param m the modulus, which must be positive
	 * @return this mod m
	 * @throws ArithmeticException if m is less than or equal to 0
	 */
	public BigInteger mod(BigInteger m){
		if (m.signum() <= 0){
			throw new ArithmeticException("modulus <= 0: "+m);
		}
		return m;
	}
	
	private static void sort(long a[], int offset, int length){
		assert a != null;
		assert offset >=0 && offset <= a.length;
		assert length >=0 && length <= a.length-offset;
		System.out.println(a);
	}
	
	public static void main(String[] args) {
		long[] a={1,2,3,4};
		sort(a, -10, -10);
	}
	
}
