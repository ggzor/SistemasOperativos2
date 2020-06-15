import * as React from "react"
import styled from "styled-components"
import { IconShell } from "./IconShell"

export const IconButton = ({ icon, text, children, ...props }) => {
  return (
    <IconButtonLayout {...props}>
      <IconShell icon={icon} />
      {children}
    </IconButtonLayout>
  )
}

const IconButtonLayout = styled.button`
  display: flex;
  align-items: center;
  padding: 4px 6px;
  user-select: none;

  background: white;
  border: 1px solid rgba(0, 0, 0, 0.55);
  border-radius: 4px;

  font-family: Roboto;
  font-size: 11px;
  line-height: 11px;

  color: rgba(0, 0, 0, 0.7);

  & > *:first-child {
    margin-right: 6px;
  }

  &:disabled {
    opacity: 0.5;
  }

  &:hover:not(:disabled) {
    background: rgba(0, 0, 0, 0.1);
  }

  &:focus {
    background: rgba(0, 0, 0, 0.05);
    outline: 0;
  }

  &:active:not(:disabled) {
    background: rgba(0, 0, 0, 0.7);
    color: white;
    outline: 0;
  }

  transition: all 100ms ease-in-out;
`
