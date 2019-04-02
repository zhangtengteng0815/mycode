package com.zhangtengteng.five;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

public class Person {
	private final Date birthDate;
	
	public Person(Date birthDate){
		this.birthDate=birthDate;
	}
	
	private static final Date BOOM_START;
	private static final Date BOOM_END;
	
	//只初始化一次
	static{
		Calendar gmtCal=Calendar.getInstance(TimeZone.getTimeZone("GMT"));
		gmtCal.set(1946, Calendar.JANUARY,1,0,0,0);
		BOOM_START=gmtCal.getTime();
		gmtCal.set(1965, Calendar.JANUARY,1,0,0,0);
		BOOM_END=gmtCal.getTime();
	}
	public boolean isBabyBoomer(){
		return birthDate.compareTo(BOOM_START)>=0&&birthDate.compareTo(BOOM_END)<0;
	}
}
