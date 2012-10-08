import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: root

    signal quit()

    showStatusBar: true
    showToolBar: true

    initialPage: RndSsuPage {}
}
