/**
*Copyright (c) 2000-2001, Jim Crafton
*All rights reserved.
*Redistribution and use in source and binary forms, with or without
*modification, are permitted provided that the following conditions
*are met:
*	Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*
*	Redistributions in binary form must reproduce the above copyright
*	notice, this list of conditions and the following disclaimer in 
*	the documentation and/or other materials provided with the distribution.
*
*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
*AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS
*OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*NB: This software will not save the world.
*/

/* Generated by Together */
#include "ApplicationKit.h"
#include "TableControl.h"
#include "DefaultTableModel.h"
#include "TableItemEditor.h"
using namespace VCF;


#define TABLECONTROL_KBRD_HANDLER		"TCKeyboardHandler"


TableControl::TableControl( TableModel* model ):
	CustomControl( true )
{
	m_model = model;
	init();
}

TableControl::~TableControl()
{
	if ( NULL != this->m_model ){
		delete m_model;
		m_model = NULL;
	}
}

void TableControl::paint( GraphicsContext * context )
{
	CustomControl::paint( context );
	

	uint32 rowCount = this->m_model->getRowCount();
	Rect itemRect;
	double x = 0;	
	double y = 0;
	int j = 0;
	for ( int i=0;i<rowCount;i++){
		TableRowItemEnumerator* rowItemEnum = m_model->getRowItemEnumerator( i );
		j = 0;
		x = 0;
		while ( rowItemEnum->hasMoreElements() ){
			TableCellItem* item = rowItemEnum->nextElement();
			if ( NULL != item ){
				itemRect.setRect( x, y, x + this->m_columnWidths[j], y + this->m_rowHeights[i] );
				item->paint( context, &itemRect );
			}
			x += this->m_columnWidths[j];
			j++;				
		}
		y += this->m_rowHeights[i];	
	}

	this->paintChildren( context );
}

void TableControl::init()
{
	m_columnResizing = false;

	m_autoResizeColumns = true;
	m_selectedCellItem = NULL;

	m_currentEditingControl = NULL;
	m_currentItemEditor = NULL;
	this->setContainer( this );	
	this->m_defaultColumnWidth = DEFAULT_COLUMN_WIDTH;
	this->m_defaultRowHeight = DEFAULT_ROW_HEIGHT;
	if ( NULL == m_model ){
		m_model = new DefaultTableModel();
	}
	TableModelEventHandler<TableControl>* tmh = 
		new TableModelEventHandler<TableControl>( this, TableControl::onTableModelChanged, "TableModelHandler" );
	
	m_model->addTableCellAddedHandler( tmh );
	m_model->addTableCellDeletedHandler( tmh );
	m_model->addTableColumnAddedHandler( tmh );
	m_model->addTableColumnDeletedHandler( tmh );
	m_model->addTableRowAddedHandler( tmh );
	m_model->addTableRowDeletedHandler( tmh );

	ItemEventHandler<TableControl>* tch = 
		new ItemEventHandler<TableControl>( this, TableControl::onTableCellItemSelected, TABLECELL_HANDLER );
	

	ModelEventHandler<TableControl>* modelHandler = 
		new ModelEventHandler<TableControl>( this, TableControl::onTableModelEmptied, "ModelHandler" );	
	m_model->addModelHandler( modelHandler );

	KeyboardEventHandler<TableControl>* kh = 
		new KeyboardEventHandler<TableControl>( this, TableControl::onEditingControlKeyPressed, TABLECONTROL_KBRD_HANDLER );
	
}

void TableControl::onTableModelEmptied( ModelEvent* e )
{
	m_columnWidths.clear();
	m_rowHeights.clear();
	m_selectedCellItem = NULL;
	m_currentEditingControl = NULL;
	m_currentItemEditor = NULL;
}

void TableControl::onTableModelChanged( TableModelEvent* event )
{
	if ( NULL != m_currentEditingControl ){		
		this->remove( m_currentEditingControl );
		delete m_currentEditingControl;		
	}
	m_selectedCellItem = NULL;
	m_currentEditingControl = NULL;
	m_currentItemEditor = NULL;
	EventHandler* itemHandler = getEventHandler(TABLECELL_HANDLER);
	switch ( event->getType() ){
		case COLUMN_DELETED:{
			
		}
		break;

		case COLUMN_ADDED:{
			this->m_columnWidths.insert( m_columnWidths.begin() + event->getColumnThatChanged(), 
				                         this->m_defaultColumnWidth );

			uint32 rowCount = this->m_model->getRowCount();
			for ( int i=0;i<rowCount;i++){
				Item* item = this->m_model->getItem( i, event->getColumnThatChanged());
				if ( NULL != item ){
					item->addItemSelectedHandler( itemHandler );
				}
			}
			
		}
		break;

		case ROW_DELETED:{
			
		}
		break;

		case ROW_ADDED:{
			this->m_rowHeights.insert( m_rowHeights.begin() + event->getRowThatChanged(),
				                       this->m_defaultRowHeight ); 

			TableRowItemEnumerator* rowItems = this->m_model->getRowItemEnumerator( event->getRowThatChanged() );
			if ( NULL != rowItems ){
				while ( true == rowItems->hasMoreElements() ){
					Item* item = rowItems->nextElement();
					if ( NULL != item ){
						item->addItemSelectedHandler( itemHandler );
					}	
				}
			}

		}
		break;

		case ALL_COLUMNS_CHANGED:{
			
		}
		break;

		case ALL_ROWS_CHANGED:{
			
		}
		break;
	}
}

uint32 TableControl::getDefaultRowHeight()
{
	return m_defaultRowHeight;
}

void TableControl::setDefaultRowHeight( const uint32& defaultRowHeight )
{
	this->m_defaultRowHeight = defaultRowHeight;
}

uint32 TableControl::getDefaultColumnWidth()
{
	return m_defaultColumnWidth;
}

void TableControl::setDefaultColumnWidth( const uint32& defaultColumnWidth )
{
	this->m_defaultColumnWidth = defaultColumnWidth;
}

void TableControl::setFixedColumn( const bool& isFixed, const uint32& col )
{
	if ( NULL != this->m_model ){
		int rowCount = this->m_model->getRowCount();
		for (int i=0;i<rowCount;i++){
			TableCellItem* item = m_model->getItem( i, col );			
			if ( NULL != item ){
				item->setFixed( isFixed );
			}
		}		
	}
}

void TableControl::setFixedRow( const bool& isFixed, const uint32& row )
{
	if ( NULL != this->m_model ){
		int colCount = this->m_model->getColumnCount();
		for (int i=0;i<colCount;i++){
			TableCellItem* item = m_model->getItem( row, i );			
			if ( NULL != item ){
				item->setFixed( isFixed );
			}
		}		
	}
}

TableModel* TableControl::getTableModel()
{
	return m_model;
}

void TableControl::setTableModel( TableModel* model )
{
	if ( NULL != m_model ){
		delete m_model;
		m_model = NULL;
	}
	m_model = model;
}

void TableControl::setColumnCount( const uint32& colCount )
{
	
}

void TableControl::setRowCount( const uint32& rowCount )
{

}

void TableControl::onTableCellItemSelected( ItemEvent* event )
{	
	if ( NULL != this->m_selectedCellItem ){
		if ( event->getSource() != m_selectedCellItem ){
			this->m_selectedCellItem->setSelected( false );
			
			this->m_selectedCellItem = dynamic_cast<TableCellItem*>(event->getSource());	
		}		
	}
	else {
		this->m_selectedCellItem = dynamic_cast<TableCellItem*>(event->getSource());
	}

	this->repaint();
}

void TableControl::mouseDown( MouseEvent* event ){
	CustomControl::mouseDown( event );	
	
	if ( NULL != m_currentEditingControl ){
		finishEditing();
	}

	uint32 rowCount = this->m_model->getRowCount();
	for ( int i=0;i<rowCount;i++){
		TableRowItemEnumerator* rowItems = this->m_model->getRowItemEnumerator( i );
		if ( NULL != rowItems ){
			while ( true == rowItems->hasMoreElements() ){
				Item* item = rowItems->nextElement();
				if ( NULL != item ){
					if ( true == item->containsPoint( event->getPoint() ) ){
						item->setSelected( true );
						break;
					}
				}
			}
		}
	}
	m_columnResizing = false;
	if ( (NULL != m_selectedCellItem) && (event->hasLeftButton()) ) {
		if ( 0 == m_selectedCellItem->getRow() ) {
			Rect itemRect = getBoundsForItem( m_selectedCellItem );
			Point* pt = event->getPoint();
			
			if ( (pt->m_x <= itemRect.m_right) && (pt->m_x > (itemRect.m_right-5)) ) {
				m_columnResizing = true;
				this->keepMouseEvents();
				m_resizeDragPt = *pt;
				m_dragColumnWidth = this->m_columnWidths[m_selectedCellItem->getColumn()];
			}
		}
	}
}

Rect TableControl::getBoundsForItem( TableCellItem* item )
{
	Rect result;	
	uint32 row = item->getRow();
	uint32 col = item->getColumn();

	uint32 rowCount = this->m_model->getRowCount();
	Rect itemRect;
	double x = 0;	
	double y = 0;
	int j = 0;
	bool finished = false;
	for ( int i=0;i<rowCount;i++){
		TableRowItemEnumerator* rowItemEnum = m_model->getRowItemEnumerator( i );
		j = 0;
		x = 0;
		while ( rowItemEnum->hasMoreElements() ){
			rowItemEnum->nextElement();
			if ( (row == i) && (col == j) ){
				result.setRect( x, y, x + this->m_columnWidths[j], y + this->m_rowHeights[i] );	
				finished = true;
				break;
			}
			x += this->m_columnWidths[j];
			j++;				
		}
		y += this->m_rowHeights[i];	
		if ( finished ) {
			break;
		}
	}
	return result;
}

void TableControl::mouseMove( MouseEvent* event ){
	CustomControl::mouseMove( event );
	if ( (NULL != m_selectedCellItem) && (event->hasLeftButton()) && (true == m_columnResizing) ) {
		uint32 deltax = event->getPoint()->m_x - m_resizeDragPt.m_x;
		this->setColumnWidth( m_selectedCellItem->getColumn(), m_dragColumnWidth + deltax );
	}
}

void TableControl::mouseUp( MouseEvent* event ){
	CustomControl::mouseUp( event );
	if ( (NULL != m_selectedCellItem) && (event->hasLeftButton()) && (true == m_columnResizing) ) {
		uint32 deltax = event->getPoint()->m_x - m_resizeDragPt.m_x;
		this->setColumnWidth( m_selectedCellItem->getColumn(), m_dragColumnWidth + deltax );
		this->releaseMouseEvents();
	}
	m_columnResizing = false;
}

void TableControl::mouseDblClick(  MouseEvent* event )
{
	CustomControl::mouseDblClick( event );
	if ( NULL != m_selectedCellItem ){
		if ( true == m_selectedCellItem->isItemEditable() ){
			m_currentItemEditor = m_selectedCellItem->getItemEditor();
			if ( NULL != m_currentItemEditor ){
				m_currentItemEditor->setItemToEdit( m_selectedCellItem );
				Control* editorControl = m_currentItemEditor->getEditingControl();
				if ( NULL != editorControl ){
					Rect bounds;
					uint32 c = m_selectedCellItem->getColumn();
					uint32 r = m_selectedCellItem->getRow();
					for ( int i=1;i<=c;i++){
						bounds.m_left +=  m_columnWidths[i-1];
					}

					for ( int j=1;j<=r;j++){
						bounds.m_top +=  m_rowHeights[j-1];
					}

					bounds.m_right = bounds.m_left + m_columnWidths[c];
					bounds.m_bottom = bounds.m_top + m_rowHeights[r];
					editorControl->setBounds( &bounds );
					
					this->add( editorControl );
					m_currentEditingControl = editorControl;
					m_currentEditingControl->setFocus( true );
					m_currentEditingControl->setVisible( true );

					EventHandler* kl = getEventHandler( TABLECONTROL_KBRD_HANDLER );
					m_currentEditingControl->addKeyPressedHandler( kl );
				}
			}
		}
	}
}

void TableControl::setColumnWidth( const uint32& column, const uint32& width )
{
	m_columnWidths[column] = width;
	uint32 colCount = m_columnWidths.size();
	if ( (true == m_autoResizeColumns) && (colCount>1) ) {
		double w = getWidth();
		int32 totwidth = 0;
		for ( int i=0;i<colCount-1;i++) {
			totwidth += m_columnWidths[i];
		}
		if ( totwidth < w ){
			m_columnWidths[colCount-1] = w-totwidth;
		}
	}
	this->repaint();
}

void TableControl::setBounds( Rect* rect, const bool& anchorDeltasNeedUpdating ) throw( InvalidPeer )
{	
	CustomControl::setBounds( rect, anchorDeltasNeedUpdating );
	uint32 colCount = m_columnWidths.size();
	if ( (true == m_autoResizeColumns) && (colCount>1) ) {		
		setColumnWidth( colCount-1, m_columnWidths[colCount-1] );
	}
}

void TableControl::setAutoResizeColumns( const bool& autoResizeColumns )
{
	this->m_autoResizeColumns = autoResizeColumns;
}

void TableControl::resetColumnWidths()
{
	
}

void TableControl::onEditingControlKeyPressed( KeyboardEvent* event )
{
	if ( VIRT_KEY_RETURN == event->getVirtualCode() ) {
		event->setConsumed( true );
		this->finishEditing();
	}
}

void TableControl::finishEditing()
{
	if ( NULL != m_currentItemEditor ){
		this->m_currentItemEditor->updateItem();
	}
	this->remove( m_currentEditingControl );
	delete m_currentEditingControl;
	m_currentEditingControl = NULL;
	m_currentItemEditor = NULL;
}