package com.zhangtengteng.two;
public class NutritionFacts2{
	private final int servingSize;
	private final int serving;
	private final int calories;
	private final int fat;
	private final int sodium;
	private final int car;
	
	public static class Builder{
		private final int servingSize;
		private final int serving;
		
		private int calories=0;
		private int fat=0;
		private int sodium=0;
		private int car=0;
		
		public Builder(int servingSize,int serving) {
			this.servingSize=servingSize;
			this.serving=servingSize;
		}
		
		public Builder calories(int val){
			calories=val;
			return this;
		}
		public Builder fat(int val){
			fat=val;
			return this;
		}
		public Builder sodium(int val){
			sodium=val;
			return this;
		}
		public Builder car(int val){
			car=val;
			return this;
		}
		public NutritionFacts2 build(){
			return new NutritionFacts2(this);
		}
	}
	private NutritionFacts2(Builder builder){
		servingSize=builder.servingSize;
		serving=builder.serving;
		calories=builder.calories;
		fat=builder.fat;
		sodium=builder.sodium;
		car=builder.car;
	}
	public static void main(String[] args) {
		NutritionFacts2 facts=new NutritionFacts2.Builder(20, 33).calories(100)
								.car(10).build();
	}
	
	
}
