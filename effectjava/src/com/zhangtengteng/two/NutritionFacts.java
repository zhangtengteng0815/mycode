package com.zhangtengteng.two;
public class NutritionFacts{
	private final int servingSize;
	private final int serving;
	private final int calories;
	private final int fat;
	private final int sodium;
	private final int car;
	
	
	public NutritionFacts(int servingSize, int serving) {
		this(servingSize,serving,0,0,0,0);
	}


	public NutritionFacts(int servingSize, int serving, int calories) {
		this(servingSize,serving,calories,0,0,0);
	}


	public NutritionFacts(int servingSize, int serving, int calories, int fat) {
		this(servingSize,serving,calories,fat,0,0);
	}


	public NutritionFacts(int servingSize, int serving, int calories, int fat,
			int sodium) {
		this(servingSize,serving,calories,fat,sodium,0);
	}


	public NutritionFacts(int servingSize, int serving, int calories, int fat,
			int sodium, int car) {
		super();
		this.servingSize = servingSize;
		this.serving = serving;
		this.calories = calories;
		this.fat = fat;
		this.sodium = sodium;
		this.car = car;
	}
	
}
