package com.zhangtengteng.thirtynine;

import java.util.Date;

public class Period {
	private final Date start;
	private final Date end;
	public Period(Date start, Date end) {
		//保护性拷贝
		this.start=new Date(start.getTime());
		this.end=new Date(end.getTime());
		if(this.start.compareTo(this.end)>0){
			throw new IllegalArgumentException(this.start+" after " +this.end);
		}
//		this.start=start;
//		this.end=end;
	}
	
	//改进
	public Date start() {
		return new Date(start.getTime());
	}
	public Date end() {
		return new Date(end.getTime());
	}
	
//	public Date start() {
//		return start;
//	}
//	public Date end() {
//		return end;
//	}

	
	@Override
	public String toString() {
		return "Period [end=" + end + ", start=" + start + ", end()=" + end()
				+ ", start()=" + start() + ", getClass()=" + getClass()
				+ ", hashCode()=" + hashCode() + ", toString()="
				+ super.toString() + "]";
	}
	public static void main(String[] args) {
		Date start=new Date();
		Date end=new Date();
		Period p=new Period(start, end);
		end.setYear(78);
		System.out.println(p);
	}
	
}
