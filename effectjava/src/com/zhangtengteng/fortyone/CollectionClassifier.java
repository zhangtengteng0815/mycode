package com.zhangtengteng.fortyone;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class CollectionClassifier {
	public static String classify(Set<?> s){
		return "Set";
	}
	public static String classify(Collection<?> s){
		return "collection";
	}
	public static String classify(List<?> s){
		return "List";
	}
	
	public static void main(String[] args) {
		Collection<?>[] collections={
				new HashMap<String, String>().values(),
				new ArrayList<BigInteger>(),
				new HashSet<String>()
		};
		for (Collection<?> c : collections) {
			System.out.println(classify(c));
		}
	}
}
