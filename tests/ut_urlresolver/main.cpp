/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "urlresolvertest.cpp"

int main(int argc, char **argv){
  UrlResolverTest urlResolverTest;

  if (QTest::qExec(&urlResolverTest, argc, argv))
    return 1;

  return 0;
}
