package chip.devicecontroller;

import java.util.ArrayList;

public class ChipTLVType {
  public enum TLVType {
    UInt,
    Int,
    String,
    ByteArray,
    Boolean,
    Float,
    Double,
    Null,
    Struct,
    Array,
    Empty;
  }

  public static interface BaseTLVType {
    TLVType type();

    <T> T value(Class<T> clazz);
  }

  public static class UIntType implements BaseTLVType {
    private long value;

    public UIntType(long value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      Object ret;
      if (clazz.equals(Integer.class)) {
        ret = (int) value;
      } else if (clazz.equals(Long.class)) {
        ret = value;
      } else {
        throw new ClassCastException();
      }

      return clazz.cast(ret);
    }

    public long value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.UInt;
    }
  }

  public static class IntType implements BaseTLVType {
    private long value;

    public IntType(long value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      Object ret;
      if (clazz.equals(Integer.class)) {
        ret = (int) value;
      } else if (clazz.equals(Long.class)) {
        ret = value;
      } else {
        throw new ClassCastException();
      }

      return clazz.cast(ret);
    }

    public long value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.Int;
    }
  }

  public static class StringType implements BaseTLVType {
    private String value;

    public StringType(String value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(String.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public String value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.String;
    }
  }

  public static class ByteArrayType implements BaseTLVType {
    private byte[] value;

    public ByteArrayType(byte[] value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(byte[].class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public byte[] value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.ByteArray;
    }
  }

  public static class BooleanType implements BaseTLVType {
    private boolean value;

    public BooleanType(boolean value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(Boolean.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public boolean value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.Boolean;
    }
  }

  public static class FloatType implements BaseTLVType {
    private float value;

    public FloatType(float value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(Float.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public float value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.Float;
    }
  }

  public static class DoubleType implements BaseTLVType {
    private double value;

    public DoubleType(double value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(Double.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public double value() {
      return value;
    }

    @Override
    public TLVType type() {
      return TLVType.Double;
    }
  }

  public static class NullType implements BaseTLVType {
    @Override
    public <T> T value(Class<T> clazz) {
      return null;
    }

    @Override
    public TLVType type() {
      return TLVType.Null;
    }
  }

  public static class StructType implements BaseTLVType {
    private ArrayList<StructElement> value;

    public StructType(ArrayList<StructElement> value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(ArrayList.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public ArrayList<StructElement> value() {
      return value;
    }

    public StructElement value(int index) {
      return value.get(index);
    }

    public int size() {
      return value.size();
    }

    @Override
    public TLVType type() {
      return TLVType.Struct;
    }
  }

  public static class StructElement {
    private long contextTagNum;
    private BaseTLVType value;

    public StructElement(long contextTagNum, BaseTLVType value) {
      this.contextTagNum = contextTagNum;
      this.value = value;
    }

    public long contextTagNum() {
      return contextTagNum;
    }

    public <T> T value(Class<T> clazz) {
      try {
        return clazz.cast(value);
      } catch (ClassCastException e) {
        return null;
      }
    }

    public BaseTLVType value() {
      return value;
    }
  }

  public interface ArrayEncodeMapMethod<T> {
    BaseTLVType map(T value);
  }

  public interface ArrayDecodeMapMethod<T> {
    T map(BaseTLVType value);
  }

  public static class ArrayType implements BaseTLVType {
    private ArrayList<BaseTLVType> value;

    public ArrayType(ArrayList<BaseTLVType> value) {
      this.value = value;
    }

    @Override
    public <T> T value(Class<T> clazz) {
      if (!clazz.equals(ArrayList.class)) {
        throw new ClassCastException();
      }

      return clazz.cast(value);
    }

    public static <T> ArrayType generateArrayType(
        ArrayList<T> rawValue, ArrayEncodeMapMethod<T> method) {
      ArrayList<BaseTLVType> ret = new ArrayList<>();
      for (T value : rawValue) {
        ret.add(method.map(value));
      }
      return new ArrayType(ret);
    }

    public <T> ArrayList<T> map(ArrayDecodeMapMethod<T> method) {
      ArrayList<T> ret = new ArrayList<>();
      for (BaseTLVType v : value) {
        ret.add(method.map(v));
      }
      return ret;
    }

    public <T> ArrayList<T> valuesAll(Class<T> clazz) {
      try {
        return this.<T>map(v -> clazz.cast(v));
      } catch (ClassCastException e) {
        return null;
      }
    }

    public BaseTLVType value(int index) {
      return value.get(index);
    }

    public int size() {
      return value.size();
    }

    @Override
    public TLVType type() {
      return TLVType.Array;
    }
  }

  public static class EmptyType implements BaseTLVType {
    public EmptyType() {}

    @Override
    public <T> T value(Class<T> clazz) {
      return null;
    }

    @Override
    public TLVType type() {
      return TLVType.Empty;
    }
  }
}
