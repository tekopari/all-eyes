/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: /home/tom/workspace/aeManager/src/org/tbrt/aemanager/IAeProxyService.aidl
 */
package org.tbrt.aemanager;
public interface IAeProxyService extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements org.tbrt.aemanager.IAeProxyService
{
private static final java.lang.String DESCRIPTOR = "org.tbrt.aemanager.IAeProxyService";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an org.tbrt.aemanager.IAeProxyService interface,
 * generating a proxy if needed.
 */
public static org.tbrt.aemanager.IAeProxyService asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = (android.os.IInterface)obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof org.tbrt.aemanager.IAeProxyService))) {
return ((org.tbrt.aemanager.IAeProxyService)iin);
}
return new org.tbrt.aemanager.IAeProxyService.Stub.Proxy(obj);
}
public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_getMessageList:
{
data.enforceInterface(DESCRIPTOR);
java.util.List<org.tbrt.aemanager.AeMessage> _result = this.getMessageList();
reply.writeNoException();
reply.writeTypedList(_result);
return true;
}
case TRANSACTION_sendAction:
{
data.enforceInterface(DESCRIPTOR);
org.tbrt.aemanager.AeMessage _arg0;
if ((0!=data.readInt())) {
_arg0 = org.tbrt.aemanager.AeMessage.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
org.tbrt.aemanager.AeMessage _result = this.sendAction(_arg0);
reply.writeNoException();
if ((_result!=null)) {
reply.writeInt(1);
_result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
}
else {
reply.writeInt(0);
}
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements org.tbrt.aemanager.IAeProxyService
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
public java.util.List<org.tbrt.aemanager.AeMessage> getMessageList() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
java.util.List<org.tbrt.aemanager.AeMessage> _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_getMessageList, _data, _reply, 0);
_reply.readException();
_result = _reply.createTypedArrayList(org.tbrt.aemanager.AeMessage.CREATOR);
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
public org.tbrt.aemanager.AeMessage sendAction(org.tbrt.aemanager.AeMessage actionMsg) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
org.tbrt.aemanager.AeMessage _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((actionMsg!=null)) {
_data.writeInt(1);
actionMsg.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_sendAction, _data, _reply, 0);
_reply.readException();
if ((0!=_reply.readInt())) {
_result = org.tbrt.aemanager.AeMessage.CREATOR.createFromParcel(_reply);
}
else {
_result = null;
}
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
}
static final int TRANSACTION_getMessageList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_sendAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
}
public java.util.List<org.tbrt.aemanager.AeMessage> getMessageList() throws android.os.RemoteException;
public org.tbrt.aemanager.AeMessage sendAction(org.tbrt.aemanager.AeMessage actionMsg) throws android.os.RemoteException;
}
