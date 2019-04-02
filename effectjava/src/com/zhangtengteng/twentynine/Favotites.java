package com.zhangtengteng.twentynine;

import java.util.HashMap;
import java.util.Map;

public class Favotites {
	private Map<Class<?>, Object> favorites=new HashMap<Class<?>, Object>();
	
	public <T> void putFavorite(Class<T> type,T instance){
		if(null==type){
			throw new NullPointerException("Type is null");
		}
		favorites.put(type, instance);
	}
	
	public <T> T getFavorite(Class<T> type){
		return type.cast(favorites.get(type));
	}
	
}
