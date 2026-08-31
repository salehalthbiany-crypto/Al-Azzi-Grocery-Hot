إصلاحات هذا الملف:
1) إصلاح خطأ C++ الأساسي: memset غير معروف بإضافة <cstring>، مع تهيئة Engine بطريقة C++ صحيحة.
2) تحسين فحص أخطاء EGL وتحرير الموارد عند الفشل.
3) إزالة الاعتماد على ndk.dir المتقادم وإضافة android.ndkVersion في app/build.gradle.
4) تحديث checkout و setup-java لتقليل تحذيرات Node 20.
5) جعل خطوة التحقق من APK تفشل بوضوح إذا لم يتم إنتاج ملف APK.
6) إزالة إنشاء local.properties من workflow لأن Gradle يستخدم android.ndkVersion.

استبدل الملفات الثلاثة في المستودع بنفس المسارات ثم شغّل GitHub Actions.
