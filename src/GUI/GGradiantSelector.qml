import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4

Item {

    id: root

    property alias gaugeWidth: gauge.width
    property alias borderColor: borderItem.borderColor
    property alias topColor: gauge.topColor
    property alias bottomColor: gauge.bottomColor
    property alias value: gauge.value
    property alias labelVisible: label.visible

    signal editingFinished;
    width: 40

    Rectangle{
        id: gauge
        property color topColor: "black"
        property color bottomColor: "white"

        property real value: 0.5

        anchors.top: parent.top
        anchors.bottom: label.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 5
        anchors.bottomMargin: 10
        width: 25

        border.width: 0
        radius: 5

        gradient: Gradient {
            GradientStop {
                id: topGradientStop
                position: 0.00;
                color: gauge.computeColor(gauge.topColor)
            }
            GradientStop {
                id: bottomGradientStop
                position: 1.00;
                color: gauge.computeColor(gauge.bottomColor)
            }
        }
        onTopColorChanged:{
            topGradientStop.color = computeColor(gauge.topColor);
            cursor.updateColor();
        }
        onBottomColorChanged:{
            bottomGradientStop.color = computeColor(gauge.bottomColor);
            cursor.updateColor();
        }

        function computeColor(color){
            var r = color.r; var g = color.g; var b=color.b;
            return Qt.rgba(Math.sqrt(r*0.7+0.3),Math.sqrt(g*0.7+0.3),Math.sqrt(b*0.7+0.3),1);
//            return color;
        }

        MouseArea{
            anchors.fill: parent

            onPressed:          computeValue(mouse.y)
            onPositionChanged:  computeValue(mouse.y)

            function computeValue(mouseY){
                var v = 1-(mouseY/parent.height);
                gauge.value = Math.min(1.0, Math.max(0.0, v));
                root.editingFinished();

            }
        }

        Rectangle{
            id: cursor

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            height: 5
            y: 1 + (parent.height-height-2)*(1-gauge.value)
            clip: true

            onYChanged: updateColor()

            color: "#888888";
            border.color: borderItem.borderColor
            border.width: 1

            function updateColor(){
                var r1 = gauge.topColor.r; var g1 = gauge.topColor.g; var b1 = gauge.topColor.b;
                var r2 = gauge.bottomColor.r; var g2 = gauge.bottomColor.g; var b2 = gauge.bottomColor.b;
                var v = gauge.value;
                var r = r1*v + r2*(1-v); var g = g1*v + g2*(1-v); var b = b1*v + b2*(1-v);
                color = Qt.rgba(r,g,b,1);
            }
        }

        Rectangle{
            id: borderItem
            property color borderColor: "#555555"

            anchors.fill: parent
            radius: parent.radius

            color: "#00FFFFFF"
            border.color: borderColor
            border.width: 2
        }
    }

    onValueChanged: label.updateText();

    TextField{
        id: label
        visible: true
        text: '0.5'

        onEditingFinished: {
            var v = parseFloat(text);
            if(!isNaN(v) && isFinite(v)){
                v = Math.max(0, Math.min(1, v));
                gauge.value = v;
                root.editingFinished();
            }else
                updateText();

            focus = false;
        }

        function updateText(){
            var v = gauge.value;
            v = Math.round(v*100);
            v = v/100;
            text = v;
        }

        height: visible?font.pixelSize:0.0
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: visible?2:0

        horizontalAlignment: TextInput.AlignHCenter;

        style: TextFieldStyle {
                id: textFieldStyle
                textColor: "#333333"
                font.pointSize: 13
                background: Rectangle {
                    radius: 2
                    implicitWidth: 100
                    implicitHeight: textFieldStyle.font.pixelSize;
                    border.color: "#00FFFFFF"
                    border.width: 0
                }
            }
    }
}
