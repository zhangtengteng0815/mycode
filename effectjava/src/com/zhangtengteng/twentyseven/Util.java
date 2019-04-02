package com.zhangtengteng.twentyseven;

import java.util.Iterator;
import java.util.List;

public class Util {
	public static <T extends Comparable<T>> T max(List<T> list){
		Iterator<T> i=list.iterator();
		T result=i.next();
		while(i.hasNext()){
			T t=i.next();
			if(t.compareTo(result)>0){
				result=t;
			}
		}
		return result;
	}
}
