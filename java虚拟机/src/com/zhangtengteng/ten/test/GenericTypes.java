package com.zhangtengteng.ten.test;

import java.util.ArrayList;
import java.util.List;
/**
 * 返回值不包含在方法的特征签名之中，所以返回值不参与重载选择
 * 但是在class文件格式之中，只要描述符不是完全一致的两个方法就可以共存
 * @author user
 */
public class GenericTypes {
	public static String method(List<String> list){
		System.out.println("List<String>");
		return "";
	}
	public static Integer method(List<Integer> list){
		System.out.println("List<Integer>");
		return 1;
	}
	public static void main(String[] args) {
		method(new ArrayList<String>());
		method(new ArrayList<Integer>());
	}
}
