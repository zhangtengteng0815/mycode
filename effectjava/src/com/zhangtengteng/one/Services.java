package com.zhangtengteng.one;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Services {
	private Services(){}
	private static final Map<String, Provider> providers=new ConcurrentHashMap<String, Provider>();
	public static final String DEFAUL_PROVIDER_NAME="<def>";
	public static void registerDefaulProvider(Provider p){
		registerDefaulProvider(DEFAUL_PROVIDER_NAME,p);
	}
	public static void registerDefaulProvider(String name,Provider p){
		providers.put(name, p);
	}
	public static Service newInstance(){
		return newInstance(DEFAUL_PROVIDER_NAME);
	}
	public static Service newInstance(String name){
		Provider  p=providers.get(name);
		if(null==p){
			throw new IllegalArgumentException();
		}
		return p.newService();
	}
}
