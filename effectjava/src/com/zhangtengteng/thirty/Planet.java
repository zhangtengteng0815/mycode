package com.zhangtengteng.thirty;

public enum Planet {
	MERCURY(3.302e+23,2.439e6),		//水星
	VENUS(4.86e+24,6.052e6),		//金星
	EARTH(5.975e+24,6.378e6),		//地球
	MARS(6.419e+23,3.39e6),			//火星
	JUPITER(1.899e+27,7.149e7),		//木星
	SATURN(5.685e+26,6.027e7),		//土星
	URANUS(8.683e+25,2.556e7),		//天王星
	NEPTUNE(1.024e+26,2.477e7);		//海王星
	private final double mass;		//质量
	private final double radius;	//半径
	private final double surfaceGravity;	//地表重力
	private static final double G=6.67300e-11;
	Planet(double mass,double radius){
		this.mass=mass;
		this.radius=radius;
		surfaceGravity=G*mass/(radius*radius);
	}
	public double mass() {
		return mass;
	}
	public double radius() {
		return radius;
	}
	public double surfaceGravity() {
		return surfaceGravity;
	}

}
