package com.zhangtengteng.fifteen;

public final class Complex {
	private final double re;
	private final double im;
	public Complex(double re,double im) {
		this.re=re;
		this.im=im;
	}
	public double getRe() {
		return re;
	}
	public double getIm() {
		return im;
	}
	
	public Complex add(Complex c){
		return new Complex(re+c.re, im+c.im);
	}
	
}
