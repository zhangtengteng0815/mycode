package com.zhangtengteng.nine.test;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public class DynamicProxyTest {
	interface IHello{
		void sayHello();
	}
	static class Hello implements IHello{
		@Override
		public void sayHello() {
			System.out.println("hello world");
		}
	}
	static class DynamicProxy implements InvocationHandler{
		Object originalObj;
		Object bind(Object obj){
			this.originalObj=obj;
			return Proxy.newProxyInstance(originalObj.getClass().getClassLoader(),
					originalObj.getClass().getInterfaces(), this);
		}
		@Override
		public Object invoke(Object proxy, Method method, Object[] args)
				throws Throwable {
			System.out.println("welcome");
			return method.invoke(originalObj, args);
		}
		public static void main(String[] args){
			/* 设置此系统属性,让JVM生成的Proxy类写入文件.保存路径为：com\zhangtengteng\nine\test\(如果不存在请手工创建) */
			System.getProperties().put("sun.misc.ProxyGenerator.saveGeneratedFiles", "true"); 
	
			IHello hello=(IHello) new DynamicProxy().bind(new Hello());
			hello.sayHello();
			
		}
		
	}
	
}
