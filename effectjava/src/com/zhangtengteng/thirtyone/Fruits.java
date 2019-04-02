package com.zhangtengteng.thirtyone;

public enum Fruits {
	Apple(1),Orange(2),Banana(3),Pineapple(4),
	Pear(5),Pitaya(6),Grape(7),Watermelon(8);
	private final int numberOfFruit;
	private Fruits(int size) {
		numberOfFruit=size;
	}
	public int numberOfFruit() {
		return numberOfFruit;
	}
	
}
