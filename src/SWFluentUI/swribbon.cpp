#include "../SWFluentUI/swribbon.hpp"

#include <QPainter>
#include <QScrollArea>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QApplication>

using namespace SWFluentUI;

// ── SWRibbonTabBar ────────────────────────────────────────────────────────────

SWRibbonTabBar::SWRibbonTabBar(QWidget *parent) : QTabBar(parent)
{
  setDrawBase(false);
  setFixedHeight(30);
}

void SWRibbonTabBar::setTabColor(int index, const QColor &color)
{
  m_colors[index] = color;
  update();
}

void SWRibbonTabBar::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // Usamos palette() del widget, no QApplication::palette(),
  // para respetar cambios de tema en vivo
  const QPalette pal = palette();

  for (int i = 0; i < count(); ++i) {
	const QRect r        = tabRect(i);
	const bool  selected = (currentIndex() == i);

	if (selected)
	  p.fillRect(r, pal.color(QPalette::Window));

	// Indicador inferior: color de contexto o color de acento del sistema
	if (m_colors.contains(i)) {
	  p.fillRect(QRect(r.left(), r.bottom() - 3, r.width(), 3), m_colors[i]);
	} else if (selected) {
	  p.fillRect(QRect(r.left(), r.bottom() - 3, r.width(), 3),
				 pal.color(QPalette::Highlight));
	}

	p.setPen(pal.color(selected ? QPalette::Text : QPalette::WindowText));
	p.drawText(r, Qt::AlignCenter, tabText(i));
  }
}

void SWRibbonTabBar::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::PaletteChange ||
	  event->type() == QEvent::ApplicationPaletteChange ||
	  event->type() == QEvent::StyleChange) {
	update();
  }
  QTabBar::changeEvent(event);
}

// ── SWRibbon ──────────────────────────────────────────────────────────────────

SWRibbon::SWRibbon(QWidget *parent) : QWidget(parent)
{
  setFixedHeight(m_normalHeight);
  m_ribbonHeight = m_normalHeight;

  // Animación sobre ribbonHeight — propiedad Q_PROPERTY dedicada
  // que llama setRibbonHeight(), evitando conflictos con el layout manager
  m_animation = new QPropertyAnimation(this, "ribbonHeight", this);
  m_animation->setDuration(200);
  m_animation->setEasingCurve(QEasingCurve::InOutQuad);

  m_topContainer = new QWidget(this);
  m_topContainer->setFixedHeight(32);

  m_quickAccess = new QToolBar(m_topContainer);
  m_quickAccess->setIconSize(QSize(16, 16));
  m_quickAccess->setStyleSheet("QToolBar { border: none; background: transparent; }");

  m_tabBar = new SWRibbonTabBar(m_topContainer);
  m_stack  = new QStackedWidget(this);

  auto *topLayout = new QHBoxLayout(m_topContainer);
  topLayout->setContentsMargins(5, 0, 5, 0);
  topLayout->addWidget(m_quickAccess);
  topLayout->addWidget(m_tabBar);
  topLayout->addStretch();

  // El toggleBtn vive en el topContainer dentro del layout
  // para que no dependa de posicionamiento manual con move()
  m_toggleBtn = new QToolButton(m_topContainer);
  m_toggleBtn->setArrowType(Qt::UpArrow);
  m_toggleBtn->setAutoRaise(true);
  m_toggleBtn->setFixedSize(20, 20);
  topLayout->addWidget(m_toggleBtn);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(m_topContainer);
  mainLayout->addWidget(m_stack);

  connect(m_tabBar,    &QTabBar::currentChanged,  this, &SWRibbon::onTabChanged);
  connect(m_toggleBtn, &QToolButton::clicked,      this, &SWRibbon::toggleMinimized);
}

// ── Alturas configurables ─────────────────────────────────────────────────────

void SWRibbon::setNormalHeight(int h)
{
  m_normalHeight = h;
  if (!m_minimized && !m_compactMode) {
	m_ribbonHeight = h;
	setFixedHeight(h);
  }
}

void SWRibbon::setCompactHeight(int h)
{
  m_compactHeight = h;
  if (!m_minimized && m_compactMode) {
	m_ribbonHeight = h;
	setFixedHeight(h);
  }
}

void SWRibbon::setMinimizedHeight(int h)
{
  m_minimizedHeight = h;
  if (m_minimized) {
	m_ribbonHeight = h;
	setFixedHeight(h);
  }
}

// ── Propiedad animable ────────────────────────────────────────────────────────

void SWRibbon::setRibbonHeight(int h)
{
  m_ribbonHeight = h;
  setFixedHeight(h);
}

// ── Minimizado / compacto ─────────────────────────────────────────────────────

void SWRibbon::toggleMinimized()
{
  setMinimized(!m_minimized);
}

void SWRibbon::setMinimized(bool minimized)
{
  if (m_minimized == minimized) return;
  m_minimized = minimized;
  m_toggleBtn->setArrowType(minimized ? Qt::DownArrow : Qt::UpArrow);
  updateHeightWithAnimation();
  emit minimizedChanged(minimized);
}

void SWRibbon::setCompactMode(bool compact)
{
  if (m_compactMode == compact) return;
  m_compactMode = compact;
  if (!m_minimized)
	updateHeightWithAnimation();
  for (int i = 0; i < m_stack->count(); ++i) {
	if (auto *page = qobject_cast<SWRibbonPage*>(m_stack->widget(i))) {
	  for (auto *group : page->groups())
		group->setCompactMode(compact);
	}
  }
  emit compactModeChanged(compact);
}

void SWRibbon::updateHeightWithAnimation()
{
  const int target = m_minimized
					   ? m_minimizedHeight
					   : (m_compactMode ? m_compactHeight : m_normalHeight);

  // Ocultamos el stack antes de animar hacia abajo para evitar flicker
  if (m_minimized)
	m_stack->setVisible(false);

  m_animation->stop();
  m_animation->setStartValue(m_ribbonHeight);
  m_animation->setEndValue(target);

  // Cuando la animación termina, mostramos el stack si se está expandiendo
  disconnect(m_animation, &QPropertyAnimation::finished, nullptr, nullptr);
  if (!m_minimized) {
	connect(m_animation, &QPropertyAnimation::finished, this, [this](){
	  m_stack->setVisible(true);
	});
  }

  m_animation->start();
}

// ── Eventos ───────────────────────────────────────────────────────────────────

void SWRibbon::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  recalcGroupCollapse();
}

void SWRibbon::recalcGroupCollapse()
{
  if (m_minimized || !m_stack->currentWidget()) return;
  auto *page = qobject_cast<SWRibbonPage*>(m_stack->currentWidget());
  if (!page) return;

  const int available = m_stack->contentsRect().width() - 30;
  int used = 0;

  for (auto *g : page->groups()) {
	// Usamos el sizeHint expandido (no colapsado) para calcular el espacio real necesario
	// Forzamos temporalmente el estado expandido para obtener el sizeHint correcto
	const bool wasCollapsed = g->property("collapsed").toBool();
	Q_UNUSED(wasCollapsed)

	const int needed = g->sizeHint().width() + 6;
	g->setCollapsed(used + needed > available);
	used += needed;
  }
}

void SWRibbon::wheelEvent(QWheelEvent *event)
{
  if (m_topContainer->geometry().contains(event->position().toPoint())) {
	const int index = m_tabBar->currentIndex();
	if (event->angleDelta().y() > 0 && index > 0)
	  m_tabBar->setCurrentIndex(index - 1);
	else if (event->angleDelta().y() < 0 && index < m_tabBar->count() - 1)
	  m_tabBar->setCurrentIndex(index + 1);
	event->accept();
  } else {
	QWidget::wheelEvent(event);
  }
}

void SWRibbon::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (m_topContainer->geometry().contains(event->position().toPoint()))
	toggleMinimized();
  QWidget::mouseDoubleClickEvent(event);
}

void SWRibbon::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::PaletteChange ||
	  event->type() == QEvent::ApplicationPaletteChange ||
	  event->type() == QEvent::StyleChange) {
	update();
  }
  QWidget::changeEvent(event);
}

void SWRibbon::onTabChanged(int index)
{
  m_stack->setCurrentIndex(index);
}

// ── Páginas y acciones ────────────────────────────────────────────────────────

SWRibbonPage* SWRibbon::addPage(const QString &title)
{
  auto *p = new SWRibbonPage(this);
  // Orden consistente: primero tab, luego stack,
  // así el índice de ambos siempre coincide
  m_tabBar->addTab(title);
  m_stack->addWidget(p);
  return p;
}

SWRibbonPage* SWRibbon::addContextPage(const QString &title, const QColor &color)
{
  auto *p   = new SWRibbonPage(this);
  const int idx = m_tabBar->addTab(title);
  m_stack->addWidget(p);
  m_tabBar->setTabColor(idx, color);
  m_tabBar->setTabVisible(idx, false);
  m_contextTabs[title] = { idx, color };
  return p;
}

void SWRibbon::setContextVisible(const QString &title, bool visible)
{
  if (!m_contextTabs.contains(title)) return;
  const auto ctx = m_contextTabs[title];
  m_tabBar->setTabVisible(ctx.index, visible);
  if (visible) {
	m_tabBar->setCurrentIndex(ctx.index);
	setMinimized(false);
  }
}

void SWRibbon::addQuickAction(QAction *action)
{
  m_quickAccess->addAction(action);
}

// ── SWRibbonPage ──────────────────────────────────────────────────────────────

SWRibbonPage::SWRibbonPage(QWidget *parent) : QWidget(parent)
{
  auto *main = new QVBoxLayout(this);
  main->setContentsMargins(0, 0, 0, 0);

  auto *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto *container = new QWidget;
  m_layout = new QHBoxLayout(container);
  m_layout->setContentsMargins(4, 2, 4, 2);
  m_layout->setSpacing(4);
  m_layout->addStretch();

  scroll->setWidget(container);
  main->addWidget(scroll);
}

SWRibbonGroup* SWRibbonPage::addGroup(const QString &title)
{
  auto *g = new SWRibbonGroup(title, this);
  // Insertamos antes del stretch final
  m_layout->insertWidget(m_layout->count() - 1, g);
  m_groups.append(g);
  return g;
}

// ── SWRibbonGroup ─────────────────────────────────────────────────────────────

SWRibbonGroup::SWRibbonGroup(const QString &title, QWidget *parent) : QFrame(parent)
{
  setFrameShape(QFrame::StyledPanel);

  m_mainLayout = new QVBoxLayout(this);
  m_mainLayout->setContentsMargins(4, 2, 4, 2);
  m_mainLayout->setSpacing(0);

  m_contentWidget  = new QWidget(this);
  m_contentLayout  = new QHBoxLayout(m_contentWidget);
  m_contentLayout->setContentsMargins(0, 0, 0, 0);
  m_contentLayout->setSpacing(2);

  m_smallLayout = new QGridLayout;
  m_smallLayout->setSpacing(1);
  m_contentLayout->addLayout(m_smallLayout);

  m_mainLayout->addWidget(m_contentWidget);

  m_titleLabel = new QLabel(title, this);
  m_titleLabel->setAlignment(Qt::AlignCenter);
  updateTitleStyle();
  m_mainLayout->addWidget(m_titleLabel);

  m_overflowButton = new QToolButton(this);
  m_overflowButton->setText(title);
  m_overflowButton->setAutoRaise(true);
  m_overflowButton->setPopupMode(QToolButton::InstantPopup);
  m_overflowMenu = new QMenu(this);
  m_overflowButton->setMenu(m_overflowMenu);
  m_mainLayout->addWidget(m_overflowButton);
  m_overflowButton->hide();
}

void SWRibbonGroup::updateTitleStyle()
{
  // Color tomado de la paleta — se adapta a claro/oscuro automáticamente
  const QColor dimColor = palette().color(QPalette::Disabled, QPalette::WindowText);
  m_titleLabel->setStyleSheet(
	QString("font-size: 9px; color: %1;").arg(dimColor.name()));
}

void SWRibbonGroup::setSmallActionRows(int rows)
{
  m_smallActionRows = qMax(1, rows);
}

void SWRibbonGroup::setCompactMode(bool compact)
{
  m_compactMode = compact;
  m_titleLabel->setVisible(!compact && !m_collapsed);
}

QSize SWRibbonGroup::sizeHint() const
{
  if (m_collapsed)
	return m_overflowButton->sizeHint() + QSize(10, 30);
  return QFrame::sizeHint();
}

void SWRibbonGroup::addLargeAction(QAction *action)
{
  auto *btn = new QToolButton(m_contentWidget);
  btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  btn->setDefaultAction(action);
  btn->setIconSize(QSize(32, 32));
  btn->setAutoRaise(true);
  btn->setFixedSize(62, 72);
  m_contentLayout->addWidget(btn);
  m_actionButtons.append(btn);
}

void SWRibbonGroup::addSmallAction(QAction *action)
{
  const int row = m_smallCount % m_smallActionRows;
  const int col = m_smallCount / m_smallActionRows;

  auto *btn = new QToolButton(m_contentWidget);
  btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  btn->setDefaultAction(action);
  btn->setAutoRaise(true);
  m_smallLayout->addWidget(btn, row, col);
  m_actionButtons.append(btn);
  m_smallCount++;
}

void SWRibbonGroup::addWidget(QWidget *w)
{
  m_contentLayout->addWidget(w);
}

void SWRibbonGroup::addSeparator()
{
  auto *sep = new QFrame(m_contentWidget);
  sep->setFrameShape(QFrame::VLine);
  // Color de separador desde la paleta — sin hardcodear rgba
  sep->setStyleSheet(
	QString("color: %1;")
	  .arg(palette().color(QPalette::Mid).name()));
  m_contentLayout->addWidget(sep);
}

void SWRibbonGroup::rebuildOverflowMenu()
{
  m_overflowMenu->clear();
  // Usamos la lista explícita m_actionButtons — sin findChildren() frágil
  for (auto *btn : m_actionButtons) {
	if (btn && btn->defaultAction())
	  m_overflowMenu->addAction(btn->defaultAction());
  }
}

void SWRibbonGroup::setCollapsed(bool collapsed)
{
  if (m_collapsed == collapsed) return;
  m_collapsed = collapsed;
  m_contentWidget->setVisible(!collapsed);
  m_titleLabel->setVisible(!collapsed && !m_compactMode);
  m_overflowButton->setVisible(collapsed);
  if (collapsed)
	rebuildOverflowMenu();
}

void SWRibbonGroup::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::PaletteChange ||
	  event->type() == QEvent::ApplicationPaletteChange ||
	  event->type() == QEvent::StyleChange) {
	updateTitleStyle();
	// Actualizamos el color del separador si existe
	for (int i = 0; i < m_contentLayout->count(); ++i) {
	  if (auto *item = m_contentLayout->itemAt(i)) {
		if (auto *sep = qobject_cast<QFrame*>(item->widget())) {
		  if (sep->frameShape() == QFrame::VLine) {
			sep->setStyleSheet(
			  QString("color: %1;")
				.arg(palette().color(QPalette::Mid).name()));
		  }
		}
	  }
	}
	update();
  }
  QFrame::changeEvent(event);
}