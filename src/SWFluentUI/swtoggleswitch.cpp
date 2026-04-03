#include "../SWFluentUI/swtoggleswitch.hpp"

#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEasingCurve>
#include <QApplication>
#include <QScreen>
#include <QAccessible>


namespace SWFluentUI {

SWToggleSwitch::SWToggleSwitch(QWidget *parent)
  : QCheckBox(parent),
	m_offset(0.0),
	m_progress(0.0),
	m_hover(false),
	m_pressed(false),
	m_duration(160)
{
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::StrongFocus);

  // Tamaño inicial: alto igual al de QPushButton (23px base), DPI aware
  const qreal scale = qApp->primaryScreen()->logicalDotsPerInch() / 96.0;
  const int h = int(23 * scale);
  const int w = int(h * 1.9);
  setFixedSize(w, h);

  // Estado inicial coherente con el valor del checkbox
  m_offset   = isChecked() ? 1.0 : 0.0;
  m_progress = isChecked() ? 1.0 : 0.0;

  setupAnimations();

  connect(this, &QCheckBox::toggled, this, [this](bool checked) {
	m_animOffset->stop();
	m_animOffset->setEndValue(checked ? 1.0 : 0.0);
	m_animOffset->start();

	m_animProgress->stop();
	m_animProgress->setEndValue(checked ? 1.0 : 0.0);
	m_animProgress->start();

	// Notifica a los lectores de pantalla
	QAccessibleValueChangeEvent ev(this, checked);
	QAccessible::updateAccessibility(&ev);
  });
}

// ── Tamaño ───────────────────────────────────────────────────────────────────

QSize SWToggleSwitch::switchSize() const
{
  return size();
}

void SWToggleSwitch::setSwitchSize(const QSize &size)
{
  setFixedSize(size);
  update();
}

QSize SWToggleSwitch::sizeHint() const
{
  return size();
}

// ── Animaciones ───────────────────────────────────────────────────────────────

void SWToggleSwitch::setupAnimations()
{
  m_animOffset = new QPropertyAnimation(this, "offset", this);
  m_animOffset->setDuration(m_duration);
  m_animOffset->setEasingCurve(QEasingCurve::InOutCubic);

  m_animProgress = new QPropertyAnimation(this, "progress", this);
  m_animProgress->setDuration(m_duration + 20);
  m_animProgress->setEasingCurve(QEasingCurve::InOutCubic);
}

int SWToggleSwitch::animationDuration() const
{
  return m_duration;
}

void SWToggleSwitch::setAnimationDuration(int ms)
{
  m_duration = ms;
  m_animOffset->setDuration(ms);
  m_animProgress->setDuration(ms + 20);
}

// ── Propiedades internas de animación ────────────────────────────────────────

qreal SWToggleSwitch::offset() const { return m_offset; }

void SWToggleSwitch::setOffset(qreal value)
{
  m_offset = value;
  update();
}

qreal SWToggleSwitch::progress() const { return m_progress; }

void SWToggleSwitch::setProgress(qreal value)
{
  m_progress = value;
  update();
}

// ── Eventos de interacción ───────────────────────────────────────────────────

void SWToggleSwitch::enterEvent(QEnterEvent *event)
{
  m_hover = true;
  update();
  QCheckBox::enterEvent(event);
}

void SWToggleSwitch::leaveEvent(QEvent *event)
{
  m_hover   = false;
  m_pressed = false;
  update();
  QCheckBox::leaveEvent(event);
}

void SWToggleSwitch::mousePressEvent(QMouseEvent *event)
{
  m_pressed = true;
  update();
  QCheckBox::mousePressEvent(event);
}

void SWToggleSwitch::mouseReleaseEvent(QMouseEvent *event)
{
  m_pressed = false;
  update();
  QCheckBox::mouseReleaseEvent(event);
}

void SWToggleSwitch::focusInEvent(QFocusEvent *event)
{
  update();
  QCheckBox::focusInEvent(event);
}

void SWToggleSwitch::focusOutEvent(QFocusEvent *event)
{
  update();
  QCheckBox::focusOutEvent(event);
}

void SWToggleSwitch::keyPressEvent(QKeyEvent *event)
{
  // Spacebar y Enter activan/desactivan el control
  if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
	setChecked(!isChecked());
	event->accept();
	return;
  }
  QCheckBox::keyPressEvent(event);
}

void SWToggleSwitch::changeEvent(QEvent *event)
{
  switch (event->type()) {
	case QEvent::PaletteChange:
	case QEvent::ApplicationPaletteChange:
	case QEvent::StyleChange:
	  update();
	  break;
	default:
	  break;
  }
  QCheckBox::changeEvent(event);
}

// ── Pintado ──────────────────────────────────────────────────────────────────

QColor SWToggleSwitch::interpolateColor(const QColor &a, const QColor &b, qreal t)
{
  return QColor(
	int(a.red()   + (b.red()   - a.red())   * t),
	int(a.green() + (b.green() - a.green()) * t),
	int(a.blue()  + (b.blue()  - a.blue())  * t)
  );
}

void SWToggleSwitch::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  const QRect    r   = rect();
  const QPalette pal = palette();

  // Colores del fondo leídos de la paleta en tiempo real
  const QColor bgOn  = isEnabled()
	  ? pal.color(QPalette::Highlight)
	  : pal.color(QPalette::Disabled, QPalette::Highlight);
  const QColor bgOff = isEnabled()
	  ? pal.color(QPalette::Mid)
	  : pal.color(QPalette::Disabled, QPalette::Mid);

  QColor bg = interpolateColor(bgOff, bgOn, m_progress);
  if (m_hover   && isEnabled()) bg = bg.lighter(110);
  if (m_pressed && isEnabled()) bg = bg.darker(110);

  // Fondo redondeado
  p.setPen(Qt::NoPen);
  p.setBrush(bg);
  p.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

  // Knob
  const int   margin = 3;
  const int   d      = r.height() - 2 * margin;
  const int   x      = margin + int((r.width() - d - 2 * margin) * m_offset);
  const QRect knob(x, margin, d, d);

  // Sombra sutil estilo Fluent
  if (isEnabled()) {
	p.setBrush(QColor(0, 0, 0, 35));
	p.drawEllipse(knob.translated(0, 1));
  }

  // Color del knob según modo claro/oscuro y estado — igual a Windows 11
  const bool isDark = pal.color(QPalette::Window).lightness() < 128;
  QColor knobColor;
  if (!isEnabled()) {
	knobColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
  } else if (isChecked()) {
	knobColor = isDark ? qRgb(39, 40, 44) : pal.color(QPalette::Midlight);
  } else {
	// knobColor = isDark ? qRgb(248, 251, 225) : pal.color(QPalette::Shadow);
	knobColor = isDark ? qRgb(240, 251, 225) : pal.color(QPalette::Text);
  }

  p.setBrush(knobColor);
  p.drawEllipse(knob);

  // Indicador de foco: anillo punteado fino, solo visible con navegación por teclado
  if (hasFocus() && focusPolicy() != Qt::NoFocus) {
	QPen focusPen(pal.color(QPalette::Highlight).lighter(130));
	focusPen.setWidth(1);
	focusPen.setStyle(Qt::DotLine);
	p.setPen(focusPen);
	p.setBrush(Qt::NoBrush);
	p.drawRoundedRect(r.adjusted(1, 1, -1, -1), r.height() / 2.0, r.height() / 2.0);
  }
}

} // namespace FluentUI