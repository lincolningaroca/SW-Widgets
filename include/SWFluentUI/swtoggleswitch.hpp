#pragma once

#include <QCheckBox>
#include <QColor>

class QPropertyAnimation;

namespace SWFluentUI {

class SWToggleSwitch : public QCheckBox
{
  Q_OBJECT

  // Propiedades internas para las animaciones — no forman parte de la API pública
  Q_PROPERTY(qreal offset   READ offset   WRITE setOffset)
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)

  // Tamaño configurable desde código o Qt Designer
  Q_PROPERTY(QSize switchSize READ switchSize WRITE setSwitchSize)

  // Duración de la animación configurable
  Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)

public:
  explicit SWToggleSwitch(QWidget *parent = nullptr);

  // Tamaño del control
  QSize switchSize() const;
  void  setSwitchSize(const QSize &size);

  // Duración de la animación en milisegundos (default: 160ms)
  int  animationDuration() const;
  void setAnimationDuration(int ms);

  // Tamaño recomendado para layouts
  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event)    override;
  void enterEvent(QEnterEvent *event)    override;
  void leaveEvent(QEvent *event)         override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void focusInEvent(QFocusEvent *event)  override;
  void focusOutEvent(QFocusEvent *event) override;
  void keyPressEvent(QKeyEvent *event)   override;
  void changeEvent(QEvent *event)        override;

private:
  // Propiedades de animación
  qreal offset()            const;
  void  setOffset(qreal value);
  qreal progress()          const;
  void  setProgress(qreal value);

  void  setupAnimations();
  QColor interpolateColor(const QColor &a, const QColor &b, qreal t);

private:
  qreal m_offset;
  qreal m_progress;

  QPropertyAnimation *m_animOffset;
  QPropertyAnimation *m_animProgress;

  bool m_hover;
  bool m_pressed;
  int  m_duration;
};

} // namespace FluentUI